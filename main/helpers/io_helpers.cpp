#include "io_helpers.h"
#include <cstdio>
#include <libdeflate.h>
#include <Windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

std::vector<std::string> get_files_with_extension_in_directory(
	const std::string& directory, const std::string& extension)
{
	WIN32_FIND_DATA find_file_data;
	const HANDLE h_find = FindFirstFile((directory + "*." + extension).c_str(),
	                                    &find_file_data);
	if (h_find == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	std::vector<std::string> paths;

	do
	{
		if (!(find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			paths.emplace_back(directory + find_file_data.cFileName);
		}
	}
	while (FindNextFile(h_find, &find_file_data) != 0);

	FindClose(h_find);

	return paths;
}

std::vector<std::string> get_files_in_subdirectories(
	const std::string& directory)
{
	std::string newdir = directory;
	if (directory.back() != '\0')
	{
		newdir.push_back('\0');
	}

	WIN32_FIND_DATA find_file_data;
	const HANDLE h_find = FindFirstFile((directory + "*").c_str(),
	                                    &find_file_data);
	if (h_find == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	std::vector<std::string> paths;
	std::string fixed_path = directory;
	do
	{
		if (strcmp(find_file_data.cFileName, ".") != 0 && strcmp(
			find_file_data.cFileName, "..") != 0)
		{
			std::string full_path = directory + find_file_data.cFileName;
			if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (directory[directory.size() - 2] == '\0')
				{
					if (directory.back() == '\\')
					{
						fixed_path.pop_back();
						fixed_path.pop_back();
					}
				}
				if (directory.back() != '\\')
				{
					fixed_path.push_back('\\');
				}
				full_path = fixed_path + find_file_data.cFileName;
				for (const auto& path : get_files_in_subdirectories(
					     full_path + "\\"))
				{
					paths.push_back(path);
				}
			} else
			{
				paths.push_back(full_path);
			}
		}
	}
	while (FindNextFile(h_find, &find_file_data) != 0);

	FindClose(h_find);

	return paths;
}

std::string strip_extension(const std::string& path)
{
	size_t i = path.find_last_of('.');

	if (i != std::string::npos)
	{
		return path.substr(0, i);
	}
	return path;
}

std::wstring strip_extension(const std::wstring& path)
{
	size_t i = path.find_last_of('.');

	if (i != std::string::npos)
	{
		return path.substr(0, i);
	}
	return path;
}

void copy_to_clipboard(HWND owner, const std::string& str)
{
	OpenClipboard(owner);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, str.size() + 1);
	if (hg)
	{
		memcpy(GlobalLock(hg), str.c_str(), str.size() + 1);
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		GlobalFree(hg);
	} else
	{
		printf("Failed to copy\n");
		CloseClipboard();
	}
}

std::wstring get_desktop_path()
{
	wchar_t path[MAX_PATH + 1] = {0};
	SHGetSpecialFolderPathW(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE);
	return path;
}

bool is_file_accessible(const std::filesystem::path& path)
{
	FILE* f = fopen(path.string().c_str(), "r");
	if (!f)
	{
		return false;
	}
	fclose(f);
	return true;
}

void vecwrite(std::vector<uint8_t>& vec, void* data, size_t len)
{
	vec.resize(vec.size() + len);
	memcpy(vec.data() + (vec.size() - len), data, len);
}

std::vector<uint8_t> read_file_buffer(const std::filesystem::path& path)
{
	FILE* f = fopen(path.string().c_str(), "rb");

	if (!f)
	{
		return {};
	}

	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> b;
	b.resize(len);

	fread(b.data(), sizeof(uint8_t), len, f);

	return b;
}

std::vector<uint8_t> auto_decompress(std::vector<uint8_t>& vec,
                                     size_t initial_size)
{
	if (vec[0] != 0x1F && vec[1] != 0x8B)
	{
		// vec is decompressed already
		printf("Already decompressed\n");
		// we need a copy, not ref
		std::vector<uint8_t> out_vec = vec;
		return out_vec;
	}

	// we dont know what the decompressed size is, so we reallocate a buffer until we find the right size lol
	size_t buf_size = initial_size;
	auto out_buf = static_cast<uint8_t*>(malloc(buf_size));
	auto decompressor = libdeflate_alloc_decompressor();
	while (true)
	{
		out_buf = static_cast<uint8_t*>(realloc(out_buf, buf_size));
		size_t actual_size = 0;
		auto result = libdeflate_gzip_decompress(
			decompressor, vec.data(), vec.size(), out_buf, buf_size,
			&actual_size);
		if (result == LIBDEFLATE_SHORT_OUTPUT || result ==
			LIBDEFLATE_INSUFFICIENT_SPACE)
		{
			printf("Buffer size of %d insufficient...\n", buf_size);
			buf_size *= 2;
			continue;
		}
		buf_size = actual_size;
		break;
	}
	libdeflate_free_decompressor(decompressor);

	printf("Found size %d...\n", buf_size);
	out_buf = static_cast<uint8_t*>(realloc(out_buf, buf_size));
	std::vector<uint8_t> out_vec;
	out_vec.resize(buf_size);
	memcpy(out_vec.data(), out_buf, buf_size);
	free(out_buf);
	return out_vec;
}

void memread(uint8_t** src, void* dest, unsigned int len)
{
	memcpy(dest, *src, len);
	*src += len;
}
