#include "FFmpegEncoder.h"

#include <assert.h>
#include <shared/Config.hpp>
#include <shared/services/LoggingService.h>
#include <view/gui/Main.h>
#include <view/gui/Loggers.h>

#include "shared/services/FrontendService.h"

bool FFmpegEncoder::start(Params params)
{
    m_params = params;
    
    const auto bufsize_video = 2048;
    const auto bufsize_audio = m_params.arate * 8;

    g_view_logger->info("[FFmpegEncoder] arate: {}, bufsize_video: {}, bufsize_audio: {}\n", m_params.arate, bufsize_video, bufsize_audio);

#define VIDEO_PIPE_NAME "\\\\.\\pipe\\mupenvideo"
#define AUDIO_PIPE_NAME "\\\\.\\pipe\\mupenaudio"

    m_video_pipe = CreateNamedPipe(
        VIDEO_PIPE_NAME,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        bufsize_video,
        bufsize_video,
        0,
        nullptr);

    if (!m_video_pipe)
    {
        return false;
    }

    m_audio_pipe = CreateNamedPipe(
        AUDIO_PIPE_NAME,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        bufsize_audio,
        bufsize_audio,
        0,
        nullptr);

    if (!m_audio_pipe)
    {
        CloseHandle(m_video_pipe);
        return false;
    }

    static char options[4096]{};
    memset(options, 0, sizeof(options));

    sprintf(options,
            g_config.ffmpeg_final_options.data(),
            m_params.width,
            m_params.height,
            m_params.fps,
            VIDEO_PIPE_NAME,
            m_params.arate,
            AUDIO_PIPE_NAME,
            m_params.path.string().data());

    g_view_logger->info("[FFmpegEncoder] Starting encode with commandline:");
    g_view_logger->info("[FFmpegEncoder] {}", options);

    if (!CreateProcess(g_config.ffmpeg_path.c_str(),
                       options,
                       nullptr,
                       nullptr,
                       FALSE,
                       NULL,
                       nullptr,
                       nullptr,
                       &m_si,
                       &m_pi)
    )
    {
        FrontendService::show_error(std::format("Could not start ffmpeg process! Does ffmpeg exist on disk at '{}'?", g_config.ffmpeg_path).c_str());
        g_view_logger->info("CreateProcess failed ({}).", GetLastError());
        CloseHandle(m_video_pipe);
        CloseHandle(m_audio_pipe);
        return false;
    }

    m_silence_buffer = static_cast<uint8_t*>(calloc(params.arate, 1));
    m_blank_buffer = static_cast<uint8_t*>(calloc(m_params.width * m_params.height * 3, 1));
    m_dropped_frames = 0;

    m_video_thread = std::thread(&FFmpegEncoder::write_video_thread, this);
    m_audio_thread = std::thread(&FFmpegEncoder::write_audio_thread, this);

    Sleep(500);

    return true;
}

bool FFmpegEncoder::stop()
{
    m_stop_thread = true;
    m_video_cv.notify_all();
    m_audio_cv.notify_all();
    DisconnectNamedPipe(m_video_pipe);
    DisconnectNamedPipe(m_audio_pipe);
    WaitForSingleObject(m_pi.hProcess, INFINITE);
    CloseHandle(m_pi.hProcess);
    CloseHandle(m_pi.hThread);
    m_audio_thread.join();
    m_video_thread.join();

    if (!this->m_video_queue.empty() || !this->m_audio_queue.empty())
    {
        FrontendService::show_warning(std::format("Capture stopped with {} video, {} audio elements remaining in queue!\nThe capture might be corrupted.", this->m_video_queue.size(), this->m_audio_queue.size()).c_str());
    }

    if (m_dropped_frames > 0)
    {
        FrontendService::show_warning(std::format("{} frames were dropped during capture to avoid out-of-memory errors.\nThe capture might contain empty frames.", m_dropped_frames).c_str());
    }

    free(m_silence_buffer);
    free(m_blank_buffer);
    return true;
}

static bool write_pipe_checked(const HANDLE pipe, const char* buffer, const unsigned buffer_size, const bool is_video)
{
    DWORD written = 0;
    const auto result = WriteFile(pipe, buffer, buffer_size, &written, nullptr);
    if (written != buffer_size || !result)
    {
        //g_view_logger->error("[FFmpegEncoder] Error writing to {} pipe, error code {}", is_video ? "video" : "audio", GetLastError());
        return false;
    }

    return true;
}

bool FFmpegEncoder::append_audio_impl(uint8_t* audio, size_t length, bool needs_free)
{
    auto buf = static_cast<uint8_t*>(malloc(length));
    memcpy(buf, audio, length);

    m_last_write_was_video = false;

    {
        std::lock_guard lock(m_audio_queue_mutex);
        this->m_audio_queue.emplace(buf, length, needs_free);
    }
    m_audio_cv.notify_one();

    return true;
}

bool FFmpegEncoder::append_video(uint8_t* image)
{
    if (m_last_write_was_video)
    {
        g_view_logger->info("[FFmpegEncoder] writing silence buffer, length {}", m_params.arate / 16);
        append_audio_impl(m_silence_buffer, m_params.arate / 16, false);
    }
    m_last_write_was_video = true;

    auto buf = static_cast<uint8_t*>(malloc(m_params.width * m_params.height * 3));

    // HACK: If we run out of memory, we start writing empty video frames
    // This can happen when encoding at high resolutions, as ffmpeg might not start processing the pipe writes in time
    if (!buf)
    {
        g_view_logger->error("[FFmpegEncoder] Out of memory, writing blank video frame");
        ++m_dropped_frames;
        
        {
            std::lock_guard lock(m_video_queue_mutex);
            m_video_queue.emplace(m_blank_buffer, false);
        }
        m_video_cv.notify_one();
        return true;
    }

    memcpy(buf, image, m_params.width * m_params.height * 3);

    {
        std::lock_guard lock(m_video_queue_mutex);
        m_video_queue.emplace(buf, true);
    }
    m_video_cv.notify_one();

    return true;
}

bool FFmpegEncoder::append_audio(uint8_t* audio, size_t length)
{
    auto buf = static_cast<uint8_t*>(malloc(length));
    memcpy(buf, audio, length);

    return append_audio_impl(buf, length, true);
}

void FFmpegEncoder::write_audio_thread()
{
    g_view_logger->trace("[FFmpegEncoder] Audio thread ready");

    while (!this->m_stop_thread)
    {
        std::unique_lock lock(m_audio_queue_mutex);
        m_audio_cv.wait(lock, [this] { return !m_audio_queue.empty() || m_stop_thread; });

        if (this->m_audio_queue.empty())
            continue;

        auto tuple = this->m_audio_queue.front();
        this->m_audio_queue.pop();
        lock.unlock();

        const auto buf = std::get<0>(tuple);
        const auto len = std::get<1>(tuple);
        const auto needs_free = std::get<2>(tuple);

        write_pipe_checked(m_audio_pipe, (char*)buf, len, false);
        if (needs_free)
        {
            free(buf);
        }
    }
}

void FFmpegEncoder::write_video_thread()
{
    g_view_logger->trace("[FFmpegEncoder] Video thread ready");

    while (!this->m_stop_thread)
    {
        std::unique_lock lock(m_video_queue_mutex);
        m_video_cv.wait(lock, [this] { return !m_video_queue.empty() || m_stop_thread; });

        if (m_video_queue.empty())
            continue;

        auto pair = this->m_video_queue.front();
        this->m_video_queue.pop();
        lock.unlock();

        write_pipe_checked(m_video_pipe, (char*)pair.first, m_params.width * m_params.height * 3, true);
        if (pair.second)
        {
            free(pair.first);
        }
    }
}
