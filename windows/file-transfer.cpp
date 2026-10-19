/* Author: m3rlinek. */
/*
 * Explorer / shell bridge. PSCP is a separate, cancellable SFTP client,
 * required to join this window's private sharing endpoint. It never receives
 * credentials and never falls back to a new SSH connection.
 */
#include "putty.h"
#include "win-gui-seat.h"
#include "file-transfer.h"
#include "file-transfer-paths.h"
#include "file-transfer-bootstrap.h"
#include "file-transfer-progress.h"
#include "session-sharing.h"
#include <commctrl.h>
#include <cstdio>
using namespace putty_transfer;
#include <shlobj.h>
#include <string>
#include <vector>
#include <algorithm>

struct TransferJob;
struct FileTransferState {
    WinGuiSeat *wgs;
    std::string cwd;
    std::vector<std::string> pending_downloads;
    std::vector<TransferJob *> jobs;
    bool prompt_pending = false;
};

struct TransferJob {
    FileTransferState *owner;
    HWND window = nullptr, output = nullptr, button = nullptr;
    HWND filename = nullptr, progress = nullptr, amount = nullptr, speed = nullptr;
    HWND details = nullptr;
    HANDLE process = nullptr, pipe = nullptr;
    std::string log;
    ProgressStream stream;
    std::wstring stage, target, staged_file;
    bool done = false;
    bool creating = true;
    bool failed = false, details_visible = false;
};

static void update_progress(TransferJob *job)
{
    if (!job->stream.received) return;
    const auto &p = job->stream.current;
    unsigned percent = p.size ? static_cast<unsigned>(100.0L * p.done / p.size) : 100;
    SetWindowTextW(job->filename, p.filename.c_str());
    SendMessageW(job->progress, PBM_SETMARQUEE, FALSE, 0);
    SetWindowLongPtrW(job->progress, GWL_STYLE,
        GetWindowLongPtrW(job->progress, GWL_STYLE) & ~PBS_MARQUEE);
    SendMessageW(job->progress, PBM_SETPOS, percent, 0);
    wchar_t text[256];
    swprintf(text, 256, L"Current file: %u%%  (%.2f / %.2f MiB)", percent,
             p.done / 1048576.0, p.size / 1048576.0);
    SetWindowTextW(job->amount, text);
    swprintf(text, 256, L"%.2f MiB/s  |  Remaining for this file: %llu:%02llu",
             p.rate / 1048576.0, static_cast<unsigned long long>(p.eta / 60),
             static_cast<unsigned long long>(p.eta % 60));
    SetWindowTextW(job->speed, text);
}

static void update_details(TransferJob *job)
{
    std::string text = job->stream.diagnostics + job->log;
    wchar_t *decoded = dup_mb_to_wc(CP_UTF8, text.c_str());
    SetWindowTextW(job->output, decoded);
    sfree(decoded);
}

static void transfer_error(HWND window, const wchar_t *message)
{
    MessageBoxW(window, message, L"PuTTY file transfer", MB_OK | MB_ICONERROR);
}

static std::wstring new_id()
{
    GUID guid;
    wchar_t text[40];
    if (FAILED(CoCreateGuid(&guid)) || !StringFromGUID2(guid, text, 40))
        return {};
    return std::wstring(text + 1, 36); /* ASCII token without GUID braces. */
}

static bool ready(FileTransferState *state)
{
    WinGuiSeat *wgs = state->wgs;
    return conf_get_bool(wgs->conf, CONF_ssh_file_transfer) &&
        conf_get_int(wgs->conf, CONF_protocol) == PROT_SSH &&
        *conf_get_str(wgs->conf, CONF_ssh_share_id) &&
        wgs->backend && backend_connected(wgs->backend) &&
        backend_sendok(wgs->backend) && !wgs->session_closed;
}

static std::wstring download_directory(Conf *conf)
{
    const char *configured = conf_get_str(conf, CONF_ssh_download_directory);
    if (*configured) {
        auto path = wide(configured);
        if (!absolute_directory(path)) return {};
        return path;
    }
    PWSTR folder = nullptr;
    std::wstring result;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_CREATE,
                                      nullptr, &folder))) {
        result = folder;
        CoTaskMemFree(folder);
    }
    return result;
}

static void finish_job(TransferJob *job, DWORD status)
{
    job->done = true;
    CloseHandle(job->process);
    job->process = nullptr;
    if (!job->stage.empty()) {
        if (status == 0 && MoveFileExW(job->staged_file.c_str(),
                                       job->target.c_str(), 0)) {
            RemoveDirectoryW(job->stage.c_str());
            job->log += "\r\nSaved to: ";
            char *s = encode_wide_string_as_utf8(job->target.c_str());
            job->log += s;
            sfree(s);
        } else {
            status = status ? status : 1;
            job->log += "\r\nDownload incomplete or destination already exists. Partial files kept in: ";
            char *s = encode_wide_string_as_utf8(job->stage.c_str());
            job->log += s;
            sfree(s);
        }
    }
    job->log += status == 0 ? "\r\nTransfer completed.\r\n" :
                             "\r\nTransfer failed or was cancelled.\r\n";
    SetWindowTextW(job->window, status == 0 ? L"PuTTY - transfer completed" :
                                            L"PuTTY - transfer failed");
    SetWindowTextW(job->button, L"Close");
    job->failed = status != 0;
    SendMessageW(job->progress, PBM_SETMARQUEE, FALSE, 0);
    SetWindowLongPtrW(job->progress, GWL_STYLE,
        GetWindowLongPtrW(job->progress, GWL_STYLE) & ~PBS_MARQUEE);
    if (!status) SendMessageW(job->progress, PBM_SETPOS, 100, 0);
    SetWindowTextW(job->amount, status == 0 ? L"Transfer completed." : L"Transfer failed or was cancelled.");
    if (!status && !job->target.empty()) SetWindowTextW(job->filename, job->target.c_str());
    SetWindowTextW(job->speed, L"");
    ShowWindow(job->details, status ? SW_SHOW : SW_HIDE);
    update_details(job);
    KillTimer(job->window, 1);
}

static void poll_job(TransferJob *job)
{
    char buffer[4096];
    DWORD available, count;
    /* A bounded poll keeps the SSH event loop responsive under output flood. */
    for (unsigned i = 0; i < 16 && PeekNamedPipe(job->pipe, nullptr, 0,
         nullptr, &available, nullptr) && available; i++) {
        if (!ReadFile(job->pipe, buffer, std::min<DWORD>(available, sizeof(buffer)),
                      &count, nullptr) || !count) break;
        job->stream.feed(buffer, count);
    }
    DWORD status;
    if (job->process && WaitForSingleObject(job->process, 0) == WAIT_OBJECT_0) {
        /* A terminated child cannot write again. Drain buffered records before
         * finalising; continue on the next timer if this tick hit its budget. */
        available = 0;
        if (!PeekNamedPipe(job->pipe, nullptr, 0, nullptr, &available, nullptr) || !available) {
            job->stream.finish();
            GetExitCodeProcess(job->process, &status);
            update_progress(job);
            finish_job(job, status);
        }
    }
    if (job->log.size() > 64000) job->log.erase(0, job->log.size() - 60000);
    if (!job->done) update_progress(job);
    if (job->details_visible) update_details(job);
}

static LRESULT CALLBACK transfer_wndproc(HWND hwnd, UINT message,
                                        WPARAM wp, LPARAM lp)
{
    auto *job = reinterpret_cast<TransferJob *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        job = static_cast<TransferJob *>(reinterpret_cast<CREATESTRUCT *>(lp)->lpCreateParams);
        job->window = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(job));
    }
    if (!job) return DefWindowProcW(hwnd, message, wp, lp);
    switch (message) {
      case WM_CREATE:
        job->filename = CreateWindowW(L"STATIC", L"Preparing transfer...", WS_CHILD | WS_VISIBLE |
            SS_PATHELLIPSIS | SS_NOPREFIX, 16, 15, 540, 26, hwnd, nullptr, nullptr, nullptr);
        job->progress = CreateWindowW(PROGRESS_CLASSW, L"", WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
            16, 51, 540, 25, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(job->progress, PBM_SETRANGE32, 0, 100);
        SendMessageW(job->progress, PBM_SETMARQUEE, TRUE, 35);
        job->amount = CreateWindowW(L"STATIC", L"Waiting for the first file...", WS_CHILD | WS_VISIBLE,
            16, 90, 540, 24, hwnd, nullptr, nullptr, nullptr);
        job->speed = CreateWindowW(L"STATIC", L"Folder transfers show progress for each file.", WS_CHILD | WS_VISIBLE,
            16, 119, 540, 24, hwnd, nullptr, nullptr, nullptr);
        job->output = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY |
            ES_AUTOVSCROLL, 16, 154, 540, 140, hwnd, nullptr, nullptr, nullptr);
        job->details = CreateWindowW(L"BUTTON", L"Show details", WS_CHILD | WS_TABSTOP,
            16, 158, 120, 28, hwnd, reinterpret_cast<HMENU>(2), nullptr, nullptr);
        job->button = CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE |
            WS_TABSTOP, 466, 158, 90, 28, hwnd, reinterpret_cast<HMENU>(1), nullptr, nullptr);
        for (HWND control : {job->filename, job->amount, job->speed, job->button, job->details, job->output})
            SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
        SetTimer(hwnd, 1, 200, nullptr);
        return 0;
      case WM_SIZE: {
        int w = LOWORD(lp), h = HIWORD(lp);
        MoveWindow(job->filename, 16, 15, (std::max)(50, w - 32), 26, TRUE);
        MoveWindow(job->progress, 16, 51, (std::max)(50, w - 32), 25, TRUE);
        MoveWindow(job->amount, 16, 90, (std::max)(50, w - 32), 24, TRUE);
        MoveWindow(job->speed, 16, 119, (std::max)(50, w - 32), 24, TRUE);
        MoveWindow(job->output, 16, 154, (std::max)(50, w - 32), (std::max)(30, h - 210), TRUE);
        MoveWindow(job->button, (std::max)(16, w - 106), (std::max)(154, h - 43), 90, 28, TRUE);
        MoveWindow(job->details, 16, (std::max)(154, h - 43), 120, 28, TRUE);
        return 0;
      }
      case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO *>(lp)->ptMinTrackSize = {520, 270};
        return 0;
      case WM_TIMER:
        if (!job->done) poll_job(job);
        return 0;
      case WM_COMMAND:
        if (LOWORD(wp) == 2 && job->failed) {
            job->details_visible = !job->details_visible;
            ShowWindow(job->output, job->details_visible ? SW_SHOW : SW_HIDE);
            SetWindowTextW(job->details, job->details_visible ? L"Hide details" : L"Show details");
            RECT rect;
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, nullptr, 0, 0, rect.right - rect.left,
                         job->details_visible ? 440 : 270, SWP_NOMOVE | SWP_NOZORDER);
            return 0;
        }
        if (LOWORD(wp) == 1) {
            if (!job->done) {
                TerminateProcess(job->process, 1);
                WaitForSingleObject(job->process, 1000);
                poll_job(job);
            } else DestroyWindow(hwnd);
        }
        return 0;
      case WM_CLOSE:
        if (!job->done) {
            std::wstring message = L"Cancel this transfer?";
            if (!job->stage.empty()) message += L"\n\nPartial download files will remain in:\n" + job->stage;
            if (MessageBoxW(hwnd, message.c_str(), L"PuTTY file transfer",
                            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
                return 0;
        }
        DestroyWindow(hwnd);
        return 0;
      case WM_NCDESTROY: {
        KillTimer(hwnd, 1);
        if (job->process) {
            TerminateProcess(job->process, 1);
            CloseHandle(job->process);
            job->process = nullptr;
        }
        if (job->pipe) { CloseHandle(job->pipe); job->pipe = nullptr; }
        auto &jobs = job->owner->jobs;
        jobs.erase(std::remove(jobs.begin(), jobs.end(), job), jobs.end());
        job->window = nullptr;
        /* CreateWindow may fail after sending WM_NCDESTROY. Its caller still
         * owns the object until creation has returned. */
        if (!job->creating) delete job;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        return DefWindowProcW(hwnd, message, wp, lp);
      }
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}

static bool launch(FileTransferState *state, const std::vector<std::wstring> &paths,
                   bool downloading, const std::wstring &stage = {},
                   const std::wstring &target = {}, const std::wstring &basename = {})
{
    if (!ready(state) || state->jobs.size() >= 4) {
        transfer_error(state->wgs->term_hwnd, L"SSH is not ready, file transfers are disabled, or four transfer windows are already open.");
        return false;
    }
    wchar_t module[32768];
    DWORD len = GetModuleFileNameW(nullptr, module, 32768);
    if (!len || len == 32768) return false;
    std::wstring executable(module, len);
    executable = executable.substr(0, executable.find_last_of(L"\\/") + 1) + L"pscp.exe";
    std::wstring command = quote(executable) + L" -sftp -batch -r -share-id " +
        quote(wide(conf_get_str(state->wgs->conf, CONF_ssh_share_id))) + L" --";
    for (const auto &path : paths) command += L" " + quote(path);
    if (command.size() >= 32767) {
        transfer_error(state->wgs->term_hwnd, L"Too many files for one transfer. Drop a smaller group or their containing folder.");
        return false;
    }
    SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
    HANDLE read = nullptr, write = nullptr;
    if (!CreatePipe(&read, &write, &sa, 0)) return false;
    SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &sa, OPEN_EXISTING, 0, nullptr);
    STARTUPINFOEXW si = {};
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdOutput = si.StartupInfo.hStdError = write;
    si.StartupInfo.hStdInput = input;
    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    std::vector<unsigned char> attributes(size);
    si.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
    bool initialized = InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &size);
    HANDLE handles[] = {write, input};
    bool configured = initialized && UpdateProcThreadAttribute(si.lpAttributeList, 0,
        PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, sizeof(handles), nullptr, nullptr);
    PROCESS_INFORMATION pi = {};
    bool started = configured && CreateProcessW(executable.c_str(), command.data(),
        nullptr, nullptr, TRUE, CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
        nullptr, nullptr, &si.StartupInfo, &pi);
    DWORD error = GetLastError();
    if (initialized) DeleteProcThreadAttributeList(si.lpAttributeList);
    CloseHandle(write);
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (!started) {
        CloseHandle(read);
        std::wstring message = L"Cannot start pscp.exe (Windows error " +
            std::to_wstring(error) + L"). Keep the matching pscp.exe beside putty.exe.";
        transfer_error(state->wgs->term_hwnd, message.c_str());
        return false;
    }
    CloseHandle(pi.hThread);
    auto *job = new TransferJob;
    job->owner = state;
    job->process = pi.hProcess;
    job->pipe = read;
    job->stage = stage;
    job->target = target;
    job->staged_file = stage + L"\\" + basename;
    state->jobs.push_back(job);
    WNDCLASSW wc = {};
    wc.lpfnWndProc = transfer_wndproc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"PuTTYFileTransfer";
    RegisterClassW(&wc);
    INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_PROGRESS_CLASS};
    InitCommonControlsEx(&controls);
    HWND hwnd = CreateWindowW(wc.lpszClassName,
        downloading ? L"PuTTY - downloading" : L"PuTTY - uploading",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 620, 270,
        state->wgs->term_hwnd, nullptr, wc.hInstance, job);
    if (!hwnd) {
        if (job->process) {
            TerminateProcess(job->process, 1);
            CloseHandle(job->process);
        }
        if (job->pipe) CloseHandle(job->pipe);
        state->jobs.erase(std::remove(state->jobs.begin(), state->jobs.end(), job), state->jobs.end());
        delete job;
        transfer_error(state->wgs->term_hwnd, L"Cannot create the transfer progress window.");
        return false;
    }
    job->creating = false;
    ShowWindow(hwnd, SW_SHOWNORMAL);
    return true;
}

static void download_request(void *ctx)
{
    auto *state = static_cast<FileTransferState *>(ctx);
    if (state->pending_downloads.empty()) { state->prompt_pending = false; return; }
    std::string remote = state->pending_downloads.front();
    state->pending_downloads.erase(state->pending_downloads.begin());
    if (!ready(state)) {
        state->pending_downloads.clear();
        state->prompt_pending = false;
        return;
    }
    std::wstring basename = wide(remote.substr(remote.find_last_of('/') + 1));
    std::wstring directory = download_directory(state->wgs->conf);
    DWORD attrs = GetFileAttributesW(directory.c_str());
    if (!safe_basename(basename) || attrs == INVALID_FILE_ATTRIBUTES ||
        !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        transfer_error(state->wgs->term_hwnd, L"Invalid Windows filename or download folder. Set an existing absolute download folder in Connection / SSH / Files.");
        state->pending_downloads.clear();
        state->prompt_pending = false;
        return;
    }
    std::wstring target = directory + L"\\" + basename;
    for (unsigned i = 1; GetFileAttributesW(target.c_str()) != INVALID_FILE_ATTRIBUTES && i < 10000; i++)
        target = directory + L"\\" + basename + L" (" + std::to_wstring(i) + L")";
    std::wstring message = L"Download this remote file or folder?\n\n" + wide(remote) +
        L"\n\nSave to:\n" + target +
        L"\n\nUses the original SSH login. Requests from nested SSH or sudo are not supported.";
    if (MessageBoxW(state->wgs->term_hwnd, message.c_str(), L"PuTTY - confirm download",
                     MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES && ready(state)) {
        std::wstring stage = directory + L"\\.putty-transfer-" + new_id();
        if (!CreateDirectoryW(stage.c_str(), nullptr))
            transfer_error(state->wgs->term_hwnd, L"Cannot create the temporary download folder.");
        else {
            /* PSCP's remote source syntax uses backslash-escaped wildcards. */
            std::string escaped;
            for (char c : remote) {
                if (c == '\\' || c == '*' || c == '?' || c == '[' || c == ']') escaped += '\\';
                escaped += c;
            }
            if (!launch(state, {L"putty-transfer:" + wide(escaped), stage}, true,
                        stage, target, basename)) RemoveDirectoryW(stage.c_str());
        }
    }
    if (!state->pending_downloads.empty()) queue_toplevel_callback(download_request, state);
    else state->prompt_pending = false;
}

Conf *win_transfer_backend_conf(WinGuiSeat *wgs)
{
    Conf *conf = conf_copy(wgs->conf);
    if (conf_get_bool(conf, CONF_ssh_file_transfer) &&
        conf_get_bool(conf, CONF_ssh_file_transfer_auto) &&
        conf_get_int(conf, CONF_protocol) == PROT_SSH &&
        !conf_get_bool(conf, CONF_ssh_subsys) &&
        !conf_get_bool(conf, CONF_ssh_no_shell) &&
        !conf_get_bool(conf, CONF_nopty) &&
        !*conf_get_str_ambi(conf, CONF_remote_cmd, nullptr)) {
        /* The backend gets its own copy; never save the generated command in
         * the user's session or inject commands into a running application. */
        conf_set_str(conf, CONF_remote_cmd, putty_transfer_bootstrap);
        conf_set_str(conf, CONF_remote_cmd2, "");
    }
    return conf;
}

void win_transfer_prepare(WinGuiSeat *wgs)
{
    win_transfer_cleanup(wgs);
    wgs->file_transfer = new FileTransferState;
    wgs->file_transfer->wgs = wgs;
    std::wstring id = new_id();
    char *utf8 = encode_wide_string_as_utf8(id.c_str());
    gui_prepare_session_sharing(wgs->conf, utf8);
    sfree(utf8);
    DragAcceptFiles(wgs->term_hwnd, TRUE);
}

void win_transfer_cleanup(WinGuiSeat *wgs)
{
    auto *state = wgs->file_transfer;
    if (!state) return;
    delete_callbacks_for_context(state);
    while (!state->jobs.empty()) DestroyWindow(state->jobs.back()->window);
    delete state;
    wgs->file_transfer = nullptr;
}

void win_transfer_drop(WinGuiSeat *wgs, HDROP drop)
{
    auto *state = wgs->file_transfer;
    std::vector<std::wstring> paths;
    UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
    if (count > 256) count = 0;
    for (UINT i = 0; i < count; i++) {
        UINT len = DragQueryFileW(drop, i, nullptr, 0);
        std::wstring path(len + 1, 0);
        DragQueryFileW(drop, i, path.data(), len + 1);
        path.resize(len);
        paths.push_back(path);
    }
    DragFinish(drop);
    if (!state || !ready(state) || state->cwd.empty() || paths.empty()) {
        transfer_error(wgs->term_hwnd, L"File transfer requires an active SSH shell and a reported current directory. Enable file transfers and automatic Bash preparation in Connection / SSH / Files, then reconnect. Drop files at the original Bash prompt (at most 256 items). Custom remote commands and other shells need their own integration.");
        return;
    }
    std::wstring message = L"Upload " + std::to_wstring(paths.size()) +
        L" file(s)/folder(s) to:\n\n" + wide(state->cwd) +
        L"\n\nExisting remote files with the same names may be overwritten.\n"
        L"Continue only at the original SSH shell prompt, outside sudo or nested SSH.";
    std::string cwd = state->cwd;
    if (MessageBoxW(wgs->term_hwnd, message.c_str(), L"PuTTY - confirm upload",
                     MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
    paths.push_back(L"putty-transfer:" + wide(cwd) + (cwd.back() == '/' ? L"" : L"/"));
    launch(state, paths, false);
}

void win_transfer_osc(TermWin *win, const char *payload)
{
    auto *wgs = container_of(win, WinGuiSeat, termwin);
    auto *state = wgs->file_transfer;
    if (!state || !ready(state)) return;
    const char *cwd_prefix = "putty-transfer;cwd;";
    const char *download_prefix = "putty-transfer;download;";
    std::string path;
    if (!strncmp(payload, cwd_prefix, strlen(cwd_prefix))) {
        /* An invalid update must clear a formerly valid destination. */
        state->cwd.clear();
        if (decode_path(payload + strlen(cwd_prefix), path)) state->cwd = path;
    } else if (!strncmp(payload, download_prefix, strlen(download_prefix)) &&
               decode_path(payload + strlen(download_prefix), path) &&
               state->pending_downloads.size() < 16) {
        state->pending_downloads.push_back(path);
        if (!state->prompt_pending) {
            state->prompt_pending = true;
            queue_toplevel_callback(download_request, state);
        }
    }
}
