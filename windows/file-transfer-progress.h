/* Author: m3rlinek. */
/* Bounded streaming decoder for the private PSCP progress channel. */
#ifndef PUTTY_FILE_TRANSFER_PROGRESS_H
#define PUTTY_FILE_TRANSFER_PROGRESS_H
#include "file-transfer-paths.h"
#include <charconv>
#include <cstdint>
#include <array>

namespace putty_transfer {
struct Progress {
    uint64_t size = 0, done = 0, rate = 0, eta = 0;
    std::wstring filename;
};
inline bool parse_progress(const std::string &line, Progress &result)
{
    const std::string prefix = "PUTTY-PROGRESS\t";
    if (line.compare(0, prefix.size(), prefix)) return false;
    Progress next;
    std::array<uint64_t *, 4> values = {&next.size, &next.done, &next.rate, &next.eta};
    size_t pos = prefix.size();
    for (auto value : values) {
        size_t end = line.find('\t', pos);
        if (end == line.npos || end == pos) return false;
        for (size_t i = pos; i < end; i++) if (line[i] < '0' || line[i] > '9') return false;
        auto parsed = std::from_chars(line.data() + pos, line.data() + end, *value);
        if (parsed.ec != std::errc{} || parsed.ptr != line.data() + end) return false;
        pos = end + 1;
    }
    if (next.done > next.size || pos == line.size() || (line.size() - pos) % 2) return false;
    std::string filename;
    auto digit = [](char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (; pos < line.size(); pos += 2) {
        int a = digit(line[pos]), b = digit(line[pos + 1]);
        if (a < 0 || b < 0 || (!a && !b)) return false;
        filename += static_cast<char>(a * 16 + b);
    }
    next.filename = wide(filename);
    if (next.filename.empty()) return false;
    for (auto &c : next.filename) if (c < 32 || c == 127) c = L' ';
    result = next;
    return true;
}

class ProgressStream {
    std::string pending;
    bool oversized = false;
public:
    Progress current;
    bool received = false;
    std::string diagnostics;
    void diagnostic(const std::string &text) {
        diagnostics += text;
        if (diagnostics.size() > 60000) diagnostics.erase(0, diagnostics.size() - 60000);
    }
    void line() {
        if (!oversized) {
            if (!pending.empty() && pending.back() == '\r') pending.pop_back();
            Progress next;
            if (parse_progress(pending, next)) { current = next; received = true; }
            else if (!pending.empty()) diagnostic(pending + "\r\n");
        }
        pending.clear(); oversized = false;
    }
    void feed(const char *data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            if (data[i] == '\n') line();
            else if (!oversized) {
                if (pending.size() < 8192) pending += data[i];
                else { oversized = true; pending.clear(); diagnostic("[Oversized output line omitted]\r\n"); }
            }
        }
    }
    void finish() { if (!pending.empty() || oversized) line(); }
};
}
#endif
