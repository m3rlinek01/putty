/* Author: m3rlinek. */
/* Windows GUI file transfers over this window's authenticated SSH connection. */
#ifndef PUTTY_WINDOWS_FILE_TRANSFER_H
#define PUTTY_WINDOWS_FILE_TRANSFER_H
struct FileTransferState;
void win_transfer_prepare(WinGuiSeat *wgs);
Conf *win_transfer_backend_conf(WinGuiSeat *wgs);
void win_transfer_cleanup(WinGuiSeat *wgs);
void win_transfer_drop(WinGuiSeat *wgs, HDROP drop);
void win_transfer_osc(TermWin *win, const char *payload);
#endif
