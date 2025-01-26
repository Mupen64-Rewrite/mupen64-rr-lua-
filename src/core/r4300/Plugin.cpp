/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// ReSharper disable CppCStyleCast
#include "Plugin.hpp"
#include <assert.h>
#include <core/r4300/rom.h>
#include <shared/Config.hpp>
#include <core/memory/memory.h>
#include <core/r4300/r4300.h>
#include <shared/services/FrontendService.h>
#include <shared/services/IOService.h>
#include <shared/services/PlatformService.h>
#include <shared/services/LoggingService.h>

CONTROL Controls[4];

GFX_INFO dummy_gfx_info;
AUDIO_INFO dummy_audio_info;
CONTROL_INFO dummy_control_info;
RSP_INFO dummy_rsp_info;
unsigned char dummy_header[0x40];

GFX_INFO gfx_info;
AUDIO_INFO audio_info;
CONTROL_INFO control_info;
RSP_INFO rsp_info;

/* dummy functions to prevent mupen from crashing if a plugin is missing */
static void __cdecl dummy_void()
{
}

static int32_t __cdecl dummy_initiateGFX(GFX_INFO Gfx_Info) { return 1; }

static int32_t __cdecl dummy_initiateAudio(AUDIO_INFO Audio_Info)
{
    return 1;
}

static void __cdecl dummy_initiateControllers(CONTROL_INFO Control_Info)
{
}

static void __cdecl dummy_aiDacrateChanged(int32_t SystemType)
{
}

static uint32_t __cdecl dummy_aiReadLength() { return 0; }

static void __cdecl dummy_aiUpdate(int32_t)
{
}

static void __cdecl dummy_controllerCommand(int32_t Control, uint8_t* Command)
{
}

static void __cdecl dummy_getKeys(int32_t Control, BUTTONS* Keys)
{
}

static void __cdecl dummy_setKeys(int32_t Control, BUTTONS Keys)
{
}

static void __cdecl dummy_readController(int32_t Control, uint8_t* Command)
{
}

static void __cdecl dummy_keyDown(uint32_t wParam, int32_t lParam)
{
}

static void __cdecl dummy_keyUp(uint32_t wParam, int32_t lParam)
{
}

static uint32_t dummy;

static void __cdecl dummy_initiateRSP(RSP_INFO Rsp_Info,
                                      uint32_t* CycleCount)
{
};

static void __cdecl dummy_fBRead(uint32_t addr)
{
};

static void __cdecl dummy_fBWrite(uint32_t addr, uint32_t size)
{
};

static void __cdecl dummy_fBGetFrameBufferInfo(void* p)
{
};

static void __cdecl dummy_readScreen(void**, int32_t*, int32_t*)
{
}


static void __cdecl dummy_moveScreen(int32_t, int32_t)
{
}

GETDLLINFO getDllInfo;
DLLCONFIG dllConfig;
DLLTEST dllTest;
DLLABOUT dllAbout;

CHANGEWINDOW changeWindow = dummy_void;
CLOSEDLL_GFX closeDLL_gfx = dummy_void;
INITIATEGFX initiateGFX = dummy_initiateGFX;
PROCESSDLIST processDList = dummy_void;
PROCESSRDPLIST processRDPList = dummy_void;
ROMCLOSED_GFX romClosed_gfx = dummy_void;
ROMOPEN_GFX romOpen_gfx = dummy_void;
SHOWCFB showCFB = dummy_void;
UPDATESCREEN updateScreen = dummy_void;
VISTATUSCHANGED viStatusChanged = dummy_void;
VIWIDTHCHANGED viWidthChanged = dummy_void;
READSCREEN readScreen;
DLLCRTFREE DllCrtFree;
MOVESCREEN moveScreen;
CAPTURESCREEN CaptureScreen;
GETVIDEOSIZE get_video_size;
READVIDEO read_video;
FBREAD fBRead = dummy_fBRead;
FBWRITE fBWrite = dummy_fBWrite;
FBGETFRAMEBUFFERINFO fBGetFrameBufferInfo = dummy_fBGetFrameBufferInfo;

AIDACRATECHANGED aiDacrateChanged = dummy_aiDacrateChanged;
AILENCHANGED aiLenChanged = dummy_void;
AIREADLENGTH aiReadLength = dummy_aiReadLength;
CLOSEDLL_AUDIO closeDLL_audio = dummy_void;
INITIATEAUDIO initiateAudio = dummy_initiateAudio;
PROCESSALIST processAList = dummy_void;
ROMCLOSED_AUDIO romClosed_audio = dummy_void;
ROMOPEN_AUDIO romOpen_audio = dummy_void;
AIUPDATE aiUpdate = dummy_aiUpdate;

CLOSEDLL_INPUT closeDLL_input = dummy_void;
CONTROLLERCOMMAND controllerCommand = dummy_controllerCommand;
GETKEYS getKeys = dummy_getKeys;
SETKEYS setKeys = dummy_setKeys;
OLD_INITIATECONTROLLERS old_initiateControllers;
INITIATECONTROLLERS initiateControllers = dummy_initiateControllers;
READCONTROLLER readController = dummy_readController;
ROMCLOSED_INPUT romClosed_input = dummy_void;
ROMOPEN_INPUT romOpen_input = dummy_void;
KEYDOWN keyDown = dummy_keyDown;
KEYUP keyUp = dummy_keyUp;

CLOSEDLL_RSP closeDLL_RSP = dummy_void;
DORSPCYCLES doRspCycles = dummy_doRspCycles;
INITIATERSP initiateRSP = dummy_initiateRSP;
ROMCLOSED_RSP romClosed_RSP = dummy_void;

#define FUNC(target, type, fallback, name) target = (type)PlatformService::get_function_in_module(handle, name); if(!target) target = fallback

void load_gfx(void* handle)
{
    FUNC(changeWindow, CHANGEWINDOW, dummy_void, "ChangeWindow");
    FUNC(closeDLL_gfx, CLOSEDLL_GFX, dummy_void, "CloseDLL");
    FUNC(dllAbout, DLLABOUT, nullptr, "DllAbout");
    FUNC(dllConfig, DLLCONFIG, nullptr, "DllConfig");
    FUNC(dllTest, DLLTEST, nullptr, "DllTest");
    FUNC(initiateGFX, INITIATEGFX, dummy_initiateGFX, "InitiateGFX");
    FUNC(processDList, PROCESSDLIST, dummy_void, "ProcessDList");
    FUNC(processRDPList, PROCESSRDPLIST, dummy_void, "ProcessRDPList");
    FUNC(romClosed_gfx, ROMCLOSED_GFX, dummy_void, "RomClosed");
    FUNC(romOpen_gfx, ROMOPEN_GFX, dummy_void, "RomOpen");
    FUNC(showCFB, SHOWCFB, dummy_void, "ShowCFB");
    FUNC(updateScreen, UPDATESCREEN, dummy_void, "UpdateScreen");
    FUNC(viStatusChanged, VISTATUSCHANGED, dummy_void, "ViStatusChanged");
    FUNC(viWidthChanged, VIWIDTHCHANGED, dummy_void, "ViWidthChanged");
    FUNC(moveScreen, MOVESCREEN, dummy_moveScreen, "MoveScreen");
    FUNC(CaptureScreen, CAPTURESCREEN, nullptr, "CaptureScreen");
    FUNC(readScreen, READSCREEN, (READSCREEN)PlatformService::get_function_in_module(handle, "ReadScreen2"), "ReadScreen");
    FUNC(get_video_size, GETVIDEOSIZE, nullptr, "mge_get_video_size");
    FUNC(read_video, READVIDEO, nullptr, "mge_read_video");
    FUNC(fBRead, FBREAD, dummy_fBRead, "FBRead");
    FUNC(fBWrite, FBWRITE, dummy_fBWrite, "FBWrite");
    FUNC(fBGetFrameBufferInfo, FBGETFRAMEBUFFERINFO, dummy_fBGetFrameBufferInfo, "FBGetFrameBufferInfo");

    // ReadScreen returns a plugin-allocated buffer which must be freed by the same CRT
    DllCrtFree = PlatformService::get_free_function_in_module(handle);

    gfx_info.hWnd = FrontendService::get_main_window_handle();
    gfx_info.hStatusBar = g_config.is_statusbar_enabled ? FrontendService::get_statusbar_handle() : nullptr;
    gfx_info.MemoryBswaped = 1;
    gfx_info.HEADER = rom;
    gfx_info.RDRAM = (uint8_t*)rdram;
    gfx_info.DMEM = (uint8_t*)SP_DMEM;
    gfx_info.IMEM = (uint8_t*)SP_IMEM;
    gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
    gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
    gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
    gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
    gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
    gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
    gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
    gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
    gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
    gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
    gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
    gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
    gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
    gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
    gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
    gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
    gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
    gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
    gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
    gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
    gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
    gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
    gfx_info.CheckInterrupts = dummy_void;
    initiateGFX(gfx_info);
}

void load_input(uint16_t version, void* handle)
{
    FUNC(closeDLL_input, CLOSEDLL_INPUT, dummy_void, "CloseDLL");
    FUNC(controllerCommand, CONTROLLERCOMMAND, dummy_controllerCommand, "ControllerCommand");
    FUNC(getKeys, GETKEYS, dummy_getKeys, "GetKeys");
    FUNC(setKeys, SETKEYS, dummy_setKeys, "SetKeys");
    if (version == 0x0101)
    {
        FUNC(initiateControllers, INITIATECONTROLLERS, dummy_initiateControllers, "InitiateControllers");
    }
    else
    {
        FUNC(old_initiateControllers, OLD_INITIATECONTROLLERS, nullptr, "InitiateControllers");
    }
    FUNC(readController, READCONTROLLER, dummy_readController, "ReadController");
    FUNC(romClosed_input, ROMCLOSED_INPUT, dummy_void, "RomClosed");
    FUNC(romOpen_input, ROMOPEN_INPUT, dummy_void, "RomOpen");
    FUNC(keyDown, KEYDOWN, dummy_keyDown, "WM_KeyDown");
    FUNC(keyUp, KEYUP, dummy_keyUp, "WM_KeyUp");

    control_info.hMainWindow = FrontendService::get_main_window_handle();
    control_info.hinst = FrontendService::get_app_instance_handle();
    control_info.MemoryBswaped = 1;
    control_info.HEADER = rom;
    control_info.Controls = Controls;
    for (auto& controller : Controls)
    {
        controller.Present = 0;
        controller.RawData = 0;
        controller.Plugin = (int32_t)ControllerExtension::None;
    }
    if (version == 0x0101)
    {
        initiateControllers(control_info);
    }
    else
    {
        old_initiateControllers(FrontendService::get_main_window_handle(), Controls);
    }
}


void load_audio(void* handle)
{
    FUNC(closeDLL_audio, CLOSEDLL_AUDIO, dummy_void, "CloseDLL");
    FUNC(aiDacrateChanged, AIDACRATECHANGED, dummy_aiDacrateChanged, "AiDacrateChanged");
    FUNC(aiLenChanged, AILENCHANGED, dummy_void, "AiLenChanged");
    FUNC(aiReadLength, AIREADLENGTH, dummy_aiReadLength, "AiReadLength");
    FUNC(initiateAudio, INITIATEAUDIO, dummy_initiateAudio, "InitiateAudio");
    FUNC(romClosed_audio, ROMCLOSED_AUDIO, dummy_void, "RomClosed");
    FUNC(romOpen_audio, ROMOPEN_AUDIO, dummy_void, "RomOpen");
    FUNC(processAList, PROCESSALIST, dummy_void, "ProcessAList");
    FUNC(aiUpdate, AIUPDATE, dummy_aiUpdate, "AiUpdate");

    audio_info.hwnd = FrontendService::get_main_window_handle();
    audio_info.hinst = FrontendService::get_app_instance_handle();
    audio_info.MemoryBswaped = 1;
    audio_info.HEADER = rom;
    audio_info.RDRAM = (uint8_t*)rdram;
    audio_info.DMEM = (uint8_t*)SP_DMEM;
    audio_info.IMEM = (uint8_t*)SP_IMEM;
    audio_info.MI_INTR_REG = &dummy; //&(MI_register.mi_intr_reg);
    audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
    audio_info.AI_LEN_REG = &(ai_register.ai_len);
    audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
    audio_info.AI_STATUS_REG = &dummy; //&(ai_register.ai_status);
    audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
    audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);

    audio_info.CheckInterrupts = dummy_void;
    initiateAudio(audio_info);
}

void load_rsp(void* handle)
{
    FUNC(closeDLL_RSP, CLOSEDLL_RSP, dummy_void, "CloseDLL");
    FUNC(doRspCycles, DORSPCYCLES, dummy_doRspCycles, "DoRspCycles");
    FUNC(initiateRSP, INITIATERSP, dummy_initiateRSP, "InitiateRSP");
    FUNC(romClosed_RSP, ROMCLOSED_RSP, dummy_void, "RomClosed");

    rsp_info.MemoryBswaped = 1;
    rsp_info.RDRAM = (uint8_t*)rdram;
    rsp_info.DMEM = (uint8_t*)SP_DMEM;
    rsp_info.IMEM = (uint8_t*)SP_IMEM;
    rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
    rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
    rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
    rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
    rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
    rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
    rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
    rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
    rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
    rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
    rsp_info.DPC_START_REG = &dpc_register.dpc_start;
    rsp_info.DPC_END_REG = &dpc_register.dpc_end;
    rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
    rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
    rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
    rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
    rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
    rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
    rsp_info.CheckInterrupts = dummy_void;
    rsp_info.ProcessDlistList = processDList;
    rsp_info.ProcessAlistList = processAList;
    rsp_info.ProcessRdpList = processRDPList;
    rsp_info.ShowCFB = showCFB;

    int32_t i = 4;
    initiateRSP(rsp_info, (uint32_t*)&i);
}

std::pair<std::wstring, std::unique_ptr<Plugin>> Plugin::create(std::filesystem::path path)
{
	uint64_t error = 0;
    void* module = PlatformService::load_library(path.wstring().c_str(), &error);

    if (module == nullptr)
    {
        return std::make_pair(std::format(L"LoadLibrary (code {})", error), nullptr);
    }

    const auto get_dll_info = (GETDLLINFO)PlatformService::get_function_in_module(
        module, "GetDllInfo");

    if (!get_dll_info)
    {
        PlatformService::free_library(module);
    	return std::make_pair(L"GetDllInfo missing", nullptr);
    }

    PLUGIN_INFO plugin_info;
    get_dll_info(&plugin_info);

    const size_t plugin_name_len = strlen(plugin_info.Name);
    while (plugin_info.Name[plugin_name_len - 1] == ' ')
    {
        plugin_info.Name[plugin_name_len - 1] = '\0';
    }

    auto plugin = std::make_unique<Plugin>();

    plugin->m_path = path;
    plugin->m_name = std::string(plugin_info.Name);
    plugin->m_type = static_cast<PluginType>(plugin_info.Type);
    plugin->m_version = plugin_info.Version;
    plugin->m_module = module;

    g_core_logger->info("[Plugin] Created plugin {}", plugin->m_name);
	return std::make_pair(L"", std::move(plugin));
}

Plugin::~Plugin()
{
    PlatformService::free_library((void*)m_module);
}

void Plugin::config()
{
    switch (m_type)
    {
    case PluginType::Video:
        {
            if (!emu_launched)
            {
                // NOTE: Since olden days, dummy render target hwnd was the statusbar.
                dummy_gfx_info.hWnd = FrontendService::get_statusbar_handle();
                dummy_gfx_info.hStatusBar = FrontendService::get_statusbar_handle();

                auto initiateGFX = (INITIATEGFX)PlatformService::get_function_in_module((void*)m_module, "InitiateGFX");
                if (initiateGFX && !initiateGFX(dummy_gfx_info))
                {
                    FrontendService::show_dialog(L"Couldn't initialize video plugin.", L"Core", FrontendService::DialogType::Information);
                }
            }

            auto dllConfig = (DLLCONFIG)PlatformService::get_function_in_module((void*)m_module, "DllConfig");
            if (dllConfig) dllConfig(FrontendService::get_plugin_config_parent_handle());

            if (!emu_launched)
            {
                auto closeDLL_gfx = (CLOSEDLL_GFX)PlatformService::get_function_in_module((void*)m_module, "CloseDLL");
                if (closeDLL_gfx) closeDLL_gfx();
            }
            break;
        }
    case PluginType::Audio:
        {
            if (!emu_launched)
            {
                auto initiateAudio = (INITIATEAUDIO)PlatformService::get_function_in_module((void*)m_module, "InitiateAudio");
                if (initiateAudio && !initiateAudio(dummy_audio_info))
                {
                	FrontendService::show_dialog(L"Couldn't initialize audio plugin.", L"Core", FrontendService::DialogType::Information);
                }
            }

            auto dllConfig = (DLLCONFIG)PlatformService::get_function_in_module((void*)m_module, "DllConfig");
            if (dllConfig) dllConfig(FrontendService::get_plugin_config_parent_handle());

            if (!emu_launched)
            {
                auto closeDLL_audio = (CLOSEDLL_AUDIO)PlatformService::get_function_in_module((void*)m_module, "CloseDLL");
                if (closeDLL_audio) closeDLL_audio();
            }
            break;
        }
    case PluginType::Input:
        {
            if (!emu_launched)
            {
                if (m_version == 0x0101)
                {
                    auto initiateControllers = (INITIATECONTROLLERS)PlatformService::get_function_in_module((void*)m_module, "InitiateControllers");
                    if (initiateControllers) initiateControllers(dummy_control_info);
                }
                else
                {
                    auto old_initiateControllers = (OLD_INITIATECONTROLLERS)PlatformService::get_function_in_module((void*)m_module, "InitiateControllers");
                    if (old_initiateControllers) old_initiateControllers(FrontendService::get_main_window_handle(), Controls);
                }
            }

            auto dllConfig = (DLLCONFIG)PlatformService::get_function_in_module((void*)m_module, "DllConfig");
            if (dllConfig) dllConfig(FrontendService::get_plugin_config_parent_handle());

            if (!emu_launched)
            {
                auto closeDLL_input = (CLOSEDLL_INPUT)PlatformService::get_function_in_module((void*)m_module, "CloseDLL");
                if (closeDLL_input) closeDLL_input();
            }
            break;
        }
    case PluginType::RSP:
        {
            if (!emu_launched)
            {
                auto initiateRSP = (INITIATERSP)PlatformService::get_function_in_module((void*)m_module, "InitiateRSP");
                uint32_t i = 0;
                if (initiateRSP) initiateRSP(dummy_rsp_info, &i);
            }

            auto dllConfig = (DLLCONFIG)PlatformService::get_function_in_module((void*)m_module, "DllConfig");
            if (dllConfig) dllConfig(FrontendService::get_plugin_config_parent_handle());

            if (!emu_launched)
            {
                auto closeDLL_RSP = (CLOSEDLL_RSP)PlatformService::get_function_in_module((void*)m_module, "CloseDLL");
                if (closeDLL_RSP) closeDLL_RSP();
            }
            break;
        }
    default:
        assert(false);
        break;
    }
}

void Plugin::test()
{
    dllTest = (DLLTEST)PlatformService::get_function_in_module((void*)m_module, "DllTest");
    if (dllTest) dllTest(FrontendService::get_plugin_config_parent_handle());
}

void Plugin::about()
{
    dllAbout = (DLLABOUT)PlatformService::get_function_in_module((void*)m_module, "DllAbout");
    if (dllAbout) dllAbout(FrontendService::get_plugin_config_parent_handle());
}

void Plugin::load_into_globals()
{
    switch (m_type)
    {
    case PluginType::Video:
        load_gfx(m_module);
        break;
    case PluginType::Audio:
        load_audio(m_module);
        break;
    case PluginType::Input:
        load_input(m_version, m_module);
        break;
    case PluginType::RSP:
        load_rsp(m_module);
        break;
    }
}

void setup_dummy_info()
{
    int32_t i;

    /////// GFX ///////////////////////////

    dummy_gfx_info.MemoryBswaped = 1;
    dummy_gfx_info.HEADER = (uint8_t*)dummy_header;
    dummy_gfx_info.RDRAM = (uint8_t*)rdram;
    dummy_gfx_info.DMEM = (uint8_t*)SP_DMEM;
    dummy_gfx_info.IMEM = (uint8_t*)SP_IMEM;
    dummy_gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    dummy_gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
    dummy_gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
    dummy_gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
    dummy_gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
    dummy_gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
    dummy_gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
    dummy_gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
    dummy_gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
    dummy_gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
    dummy_gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
    dummy_gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
    dummy_gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
    dummy_gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
    dummy_gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
    dummy_gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
    dummy_gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
    dummy_gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
    dummy_gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
    dummy_gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
    dummy_gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
    dummy_gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
    dummy_gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
    dummy_gfx_info.CheckInterrupts = dummy_void;

    /////// AUDIO /////////////////////////
    dummy_audio_info.hwnd = FrontendService::get_main_window_handle();
    dummy_audio_info.hinst = FrontendService::get_app_instance_handle();
    dummy_audio_info.MemoryBswaped = 1;
    dummy_audio_info.HEADER = (uint8_t*)dummy_header;
    dummy_audio_info.RDRAM = (uint8_t*)rdram;
    dummy_audio_info.DMEM = (uint8_t*)SP_DMEM;
    dummy_audio_info.IMEM = (uint8_t*)SP_IMEM;
    dummy_audio_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    dummy_audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
    dummy_audio_info.AI_LEN_REG = &(ai_register.ai_len);
    dummy_audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
    dummy_audio_info.AI_STATUS_REG = &(ai_register.ai_status);
    dummy_audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
    dummy_audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);
    dummy_audio_info.CheckInterrupts = dummy_void;

    ///// CONTROLS ///////////////////////////
    dummy_control_info.hMainWindow = FrontendService::get_main_window_handle();
    dummy_control_info.hinst = FrontendService::get_app_instance_handle();
    dummy_control_info.MemoryBswaped = 1;
    dummy_control_info.HEADER = (uint8_t*)dummy_header;
    dummy_control_info.Controls = Controls;
    for (i = 0; i < 4; i++)
    {
        Controls[i].Present = 0;
        Controls[i].RawData = 0;
        Controls[i].Plugin = (int32_t)ControllerExtension::None;
    }

    //////// RSP /////////////////////////////
    dummy_rsp_info.MemoryBswaped = 1;
    dummy_rsp_info.RDRAM = (uint8_t*)rdram;
    dummy_rsp_info.DMEM = (uint8_t*)SP_DMEM;
    dummy_rsp_info.IMEM = (uint8_t*)SP_IMEM;
    dummy_rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
    dummy_rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
    dummy_rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
    dummy_rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
    dummy_rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
    dummy_rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
    dummy_rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
    dummy_rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
    dummy_rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
    dummy_rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
    dummy_rsp_info.DPC_START_REG = &dpc_register.dpc_start;
    dummy_rsp_info.DPC_END_REG = &dpc_register.dpc_end;
    dummy_rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
    dummy_rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
    dummy_rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
    dummy_rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
    dummy_rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
    dummy_rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
    dummy_rsp_info.CheckInterrupts = dummy_void;
    dummy_rsp_info.ProcessDlistList = processDList;
    dummy_rsp_info.ProcessAlistList = processAList;
    dummy_rsp_info.ProcessRdpList = processRDPList;
    dummy_rsp_info.ShowCFB = showCFB;
}
