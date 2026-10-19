/* Author: m3rlinek. */
/* Pure path/argument validation shared by the GUI bridge and its tests. */
#ifndef PUTTY_FILE_TRANSFER_PATHS_H
#define PUTTY_FILE_TRANSFER_PATHS_H
#include <windows.h>
#include <string>
#include <cstring>
#include <cwctype>
#include <algorithm>

namespace putty_transfer {
inline std::wstring wide(const std::string &s)
{
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(),
                              static_cast<int>(s.size()), nullptr, 0);
    if (!n) return {};
    std::wstring result(n, 0);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(),
                        static_cast<int>(s.size()), result.data(), n);
    return result;
}

/* Windows argument quoting, including runs of backslashes before quotes. */
inline std::wstring quote(const std::wstring &s)
{
    std::wstring out = L"\"";
    size_t slashes = 0;
    for (wchar_t c : s) {
        if (c == L'\\') { slashes++; continue; }
        out.append(slashes * (c == L'\"' ? 2 : 1), L'\\');
        slashes = 0;
        if (c == L'\"') out += L'\\';
        out += c;
    }
    out.append(slashes * 2, L'\\');
    return out + L'\"';
}

inline bool decode_path(const char *hex, std::string &path)
{
    path.clear();
    size_t len = strlen(hex);
    if (!len || len % 2 || len > 1900) return false;
    auto digit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    std::string decoded;
    for (size_t i = 0; i < len; i += 2) {
        int a = digit(hex[i]), b = digit(hex[i + 1]);
        if (a < 0 || b < 0) return false;
        unsigned c = a * 16 + b;
        if (c < 32 || c == 127) return false;
        decoded += static_cast<char>(c);
    }
    if (decoded[0] != '/' || wide(decoded).empty()) return false;
    path = decoded;
    return true;
}

inline bool safe_basename(const std::wstring &s)
{
    if (s.empty() || s == L"." || s == L".." || s.back() == L'.' ||
        s.back() == L' ' || s.find_first_of(L"<>:\"/\\|?*") != s.npos)
        return false;
    for (wchar_t c : s) if (c < 32 || c == 127) return false;
    std::wstring stem = s.substr(0, s.find(L'.'));
    /* Win32 ignores spaces before the extension when recognising devices. */
    while (!stem.empty() && stem.back() == L' ') stem.pop_back();
    std::transform(stem.begin(), stem.end(), stem.begin(), towupper);
    if (stem == L"CON" || stem == L"PRN" || stem == L"AUX" || stem == L"NUL" ||
        stem == L"CONIN$" || stem == L"CONOUT$") return false;
    if (stem.size() == 4 && (stem.substr(0, 3) == L"COM" ||
        stem.substr(0, 3) == L"LPT") &&
        ((stem[3] >= L'0' && stem[3] <= L'9') ||
         stem[3] == L'\u00b9' || stem[3] == L'\u00b2' || stem[3] == L'\u00b3'))
        return false;
    return true;
}

inline bool absolute_directory(const std::wstring &s)
{
    for (wchar_t c : s) if (c < 32 || c == 127) return false;
    if (s.size() >= 3 && ((s[0] >= L'A' && s[0] <= L'Z') ||
                         (s[0] >= L'a' && s[0] <= L'z')) &&
        s[1] == L':' && (s[2] == L'\\' || s[2] == L'/')) return true;
    /* Accept ordinary UNC server/share paths, excluding device namespaces. */
    if (s.size() < 5 || s.substr(0, 2) != L"\\\\" ||
        s[2] == L'.' || s[2] == L'?') return false;
    size_t separator = s.find(L'\\', 2);
    return separator != s.npos && separator > 2 && separator + 1 < s.size() &&
        s[separator + 1] != L'\\';
}
}
#endif
