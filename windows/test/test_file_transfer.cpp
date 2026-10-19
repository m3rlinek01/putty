/* Author: m3rlinek. */
/* Exercise actual bridge validation and PuTTY's actual Unicode argv parser. */
#include "putty.h"
#include "../file-transfer-paths.h"
#include "../file-transfer-progress.h"
#include <cstdio>
#include <vector>

using namespace putty_transfer;
void modalfatalbox(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(1);
}
static int checks;
static void check(bool result, const char *label)
{
    checks++;
    if (!result) {
        fprintf(stderr, "File transfer check failed: %s\n", label);
        exit(1);
    }
}

int main()
{
    const std::string progress_line = "PUTTY-PROGRESS\t1048576\t524288\t262144\t2\t61c5bc2e7a6970";
    Progress progress;
    check(parse_progress(progress_line, progress) && progress.size == 1048576 &&
          progress.done == 524288 && progress.rate == 262144 && progress.eta == 2 &&
          progress.filename == L"a\u017c.zip", "structured progress decode");
    for (const char *bad : {
        "PUTTY-PROGRESS\t1\t2\t0\t0\t61", "PUTTY-PROGRESS\t-1\t0\t0\t0\t61",
        "PUTTY-PROGRESS\t+1\t0\t0\t0\t61", "PUTTY-PROGRESS\t1\t0\t0\t0\t00",
        "PUTTY-PROGRESS\t18446744073709551616\t0\t0\t0\t61",
        "PUTTY-PROGRESS\t1\t0\t0\t0\tc080", "PUTTY-PROGRESS\t1\t0\t0\t0\t6",
        "PUTTY-PROGRESS\t1\t0\t0\t0\tgg", "PUTTY-PROGRESS\t1\t0\t0\t0\t61\textra",
        "PUTTY-PROGRESS\t1\t0\t\t0\t61", "PUTTY-PROGRESS\t1\t0\t0\t0\t"})
        check(!parse_progress(bad, progress), "malformed progress rejected");
    check(parse_progress("PUTTY-PROGRESS\t0\t0\t0\t0\t61", progress), "empty file progress");
    check(parse_progress("PUTTY-PROGRESS\t18446744073709551615\t18446744073709551615\t0\t0\t61", progress), "64-bit progress range");
    for (size_t split = 0; split <= progress_line.size() + 1; split++) {
        ProgressStream stream;
        std::string framed = progress_line + "\n";
        stream.feed(framed.data(), split);
        stream.feed(framed.data() + split, framed.size() - split);
        check(stream.received && stream.current.done == 524288 && stream.diagnostics.empty(), "chunk boundary progress");
    }
    ProgressStream stream;
    for (char c : progress_line + "\r\nPermission denied\n") stream.feed(&c, 1);
    check(stream.received && stream.diagnostics == "Permission denied\r\n", "mixed diagnostics and byte chunks");
    std::string flood(100000, 'x');
    stream.feed(flood.data(), flood.size());
    stream.feed("\n", 1);
    stream.feed(progress_line.data(), progress_line.size());
    stream.finish();
    check(stream.received && stream.diagnostics.size() < 60000 &&
          stream.diagnostics.find("Oversized") != std::string::npos, "bounded oversized line recovery");
    for (unsigned i = 0; i < 100000; i++) stream.feed("error\n", 6);
    check(stream.diagnostics.size() <= 60000, "bounded diagnostics memory");
    const std::vector<std::wstring> arguments = {
        L"", L"plain", L"C:\\with space\\", L"a\"b", L"a\\\"b", L"\\\\server\\share\\",
        L"\\\\", L"';&$()", L"--looks-like-option", L"/literal[1]*?.zip",
        L"C:\\Za\u017c\u00f3\u0142\u0107\\\u6587\u4ef6.zip"};
    std::wstring command = quote(L"pscp.exe");
    for (const auto &argument : arguments) command += L" " + quote(argument);
    int argc;
    wchar_t **argv;
    split_into_argv_w(command.c_str(), true, &argc, &argv, nullptr);
    check(argc == static_cast<int>(arguments.size() + 1), "quoted argv count");
    for (size_t i = 0; i < arguments.size(); i++)
        check(arguments[i] == argv[i + 1], "Unicode argv roundtrip");
    sfree(argv[0]);
    sfree(argv);
    /* Exhaust runs of backslashes immediately adjacent to embedded quotes. */
    for (unsigned n = 0; n < 12; n++) {
        std::wstring value = L"prefix " + std::wstring(n, L'\\') + L"\"tail" +
            std::wstring(n, L'\\');
        std::wstring line = L"pscp.exe " + quote(value);
        split_into_argv_w(line.c_str(), true, &argc, &argv, nullptr);
        check(argc == 2 && value == argv[1], "slash/quote roundtrip");
        sfree(argv[0]); sfree(argv);
    }
    std::string decoded;
    check(decode_path("2f746d702f615b315d2a3f2e7a6970", decoded) &&
          decoded == "/tmp/a[1]*?.zip", "literal wildcard decoded unchanged");
    check(decode_path("2FC5BCC3B3C582C487", decoded) &&
          wide(decoded) == L"/\u017c\u00f3\u0142\u0107", "UTF-8 decode");
    check(decode_path("2f", decoded) && decoded == "/", "root path");
    for (const char *bad : {"", "2", "GG", "6162", "2f00", "2f0a", "2f7f",
                            "2fc080", "2feda080", "2ff4908080", "2fc3"}) {
        decoded = "stale";
        check(!decode_path(bad, decoded) && decoded.empty(), "invalid path clears output");
    }
    std::string too_long(1902, 'a');
    check(!decode_path(too_long.c_str(), decoded), "OSC path length bound");
    for (const wchar_t *good : {L"aaa.zip", L"report (2).zip", L".hidden",
                                L"com10", L"NUL-safe.txt", L"\u6587\u4ef6.zip"})
        check(safe_basename(good), "safe Windows basename");
    for (const wchar_t *bad : {L"", L".", L"..", L"x.", L"x ", L"a:b", L"a/b",
                               L"a\\b", L"a*", L"a?", L"a|", L"a\"", L"a<", L"a>",
                               L"a\n", L"CON", L"nul.txt", L"NUL .txt", L"AUX",
                               L"PRN", L"COM1.zip", L"lpt9", L"COM\u00b9.txt",
                               L"LPT\u00b2", L"COM\u00b3", L"CONIN$", L"CONOUT$"})
        check(!safe_basename(bad), "unsafe Windows basename");
    check(!safe_basename(std::wstring(L"a\0b", 3)), "embedded basename NUL");
    for (const wchar_t *good : {L"C:\\", L"d:/Downloads", L"\\\\server\\share",
                                L"\\\\server\\share\\folder"})
        check(absolute_directory(good), "absolute download directory");
    for (const wchar_t *bad : {L"", L"Downloads", L"C:Downloads", L"\\Downloads",
                               L"\\\\server", L"\\\\server\\", L"\\\\.\\NUL",
                               L"\\\\?\\C:\\folder", L"C:\\bad\n"})
        check(!absolute_directory(bad), "relative/device directory rejected");
    printf("File transfer helpers: %d checks passed.\n", checks);
    return 0;
}
