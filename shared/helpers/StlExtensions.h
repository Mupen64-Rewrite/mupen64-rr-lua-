#pragma once

#include <algorithm>
#include <locale>
#include <cctype>
#include <vector>
#include <lib/spdlog/spdlog.h>

/**
 * \brief Records the execution time of a scope
 */
class ScopeTimer
{
public:
    ScopeTimer(const std::string& name, const std::shared_ptr<spdlog::logger>& logger)
    {
        m_name = name;
        m_logger = logger;
        m_start_time = std::chrono::high_resolution_clock::now();
    }

    ~ScopeTimer()
    {
        print_duration();
    }

    void print_duration() const
    {
        m_logger->info("[ScopeTimer] {}: {}ms", m_name.c_str(), static_cast<int>((std::chrono::high_resolution_clock::now() - m_start_time).count() / 1'000'000));
    }

private:
    std::string m_name;
    std::shared_ptr<spdlog::logger> m_logger;
    std::chrono::time_point<std::chrono::steady_clock> m_start_time;
};

static bool ichar_equals(wchar_t a, wchar_t b)
{
    return std::tolower(a) == std::tolower(b);
}

static bool iequals(std::wstring_view lhs, std::wstring_view rhs)
{
    return std::ranges::equal(lhs, rhs, ichar_equals);
}

static std::string to_lower(std::string a)
{
    std::ranges::transform(a, a.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    return a;
}

static bool contains(const std::string& a, const std::string& b)
{
    return a.find(b) != std::string::npos;
}

/**
 * Erases the specified indicies in a vector.
 * \tparam T The vector's contained type.
 * \param data The vector.
 * \param indices_to_delete The indices to delete.
 * \return The vector with the indicies removed.
 */
template <typename T>
std::vector<T> erase_indices(const std::vector<T>& data, std::vector<size_t>& indices_to_delete)
{
    if (indices_to_delete.empty())
        return data;

    std::vector<T> ret = data;

    std::ranges::sort(indices_to_delete, std::greater<>());
    for (auto i : indices_to_delete)
    {
        if (i >= ret.size())
        {
            continue;
        }
        ret.erase(ret.begin() + i);
    }

    return ret;
}


/**
 * Converts a string to a wstring. Returns an empty string on fail.
 * \param str The string to convert
 * \return The string's wstring representation
 */
static std::wstring string_to_wstring(const std::string& str)
{
    const auto wstr = static_cast<wchar_t*>(calloc(str.length(), sizeof(wchar_t) + 1));

    if (!wstr)
    {
        // Is it appropriate to fail silently...
        return L"";
    }

    mbstowcs(wstr, str.c_str(), str.length());

    auto wstdstr = std::wstring(wstr);
    wstdstr.resize(str.length());

    free(wstr);
    return wstdstr;
}

/**
 * Converts a wstring to a string.
 * \param wstr The wstring to convert
 * \return The wstring's string representation
 */
static std::string wstring_to_string(const std::wstring& wstr)
{
    std::string str(wstr.length(), 0);
    std::transform(wstr.begin(), wstr.end(), str.begin(), [](wchar_t c)
    {
        return (char)c;
    });
    return str;
}

/**
 * Splits a string by a delimiter
 * \param s The string to split
 * \param delimiter The delimiter
 * \return A vector of string segments split by the delimiter
 * \remark See https://stackoverflow.com/a/46931770/14472122
 */
static std::vector<std::wstring> split_string(const std::wstring& s, const std::wstring& delimiter)
{
    size_t pos_start = 0, pos_end;
    const size_t delim_len = delimiter.length();
    std::vector<std::wstring> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::wstring::npos)
    {
        auto token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.emplace_back(s.substr(pos_start));
    return res;
}

// FIXME: Use template...

/**
 * Splits a wstring by a delimiter
 * \param s The wstring to split
 * \param delimiter The delimiter
 * \return A vector of wstring segments split by the delimiter
 * \remark See https://stackoverflow.com/a/46931770/14472122
 */
static std::vector<std::wstring> split_wstring(const std::wstring& s, const std::wstring& delimiter)
{
    size_t pos_start = 0, pos_end;
    const size_t delim_len = delimiter.length();
    std::vector<std::wstring> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::wstring::npos)
    {
        std::wstring token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.emplace_back(s.substr(pos_start));
    return res;
}

/**
 * Writes a nul terminator at the last space in the string
 * \param str The string to trim
 * \param len The string's length including the nul terminator
 */
static void strtrim(char* str, size_t len)
{
    for (int i = 0; i < len; ++i)
    {
        if (i == 0)
        {
            continue;
        }
        if (str[i - 1] == ' ' && str[i] == ' ')
        {
            memset(str + i - 1, 0, len - i + 1);
            return;
        }
    }
}

/**
 * Finds the nth occurence of a pattern inside a string
 * \param str The string to search in
 * \param searched The pattern to search for
 * \param nth The occurence index
 * \return The start index of the occurence into the string, or std::string::npos if none was found
 */
static size_t str_nth_occurence(const std::string& str, const std::string& searched, size_t nth)
{
    if (searched.empty() || nth <= 0)
    {
        return std::string::npos;
    }

    size_t pos = 0;
    int count = 0;

    while (count < nth)
    {
        pos = str.find(searched, pos);
        if (pos == std::string::npos)
        {
            return std::string::npos;
        }
        count++;
        if (count < nth)
        {
            pos += searched.size();
        }
    }

    return pos;
}
