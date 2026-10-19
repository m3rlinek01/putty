# Shell file transfers

Requires this build's `putty.exe`, the bundled `pscp.exe` beside it, and an SSH
server with SFTP support. Transfer settings are under **Connection > SSH > Files**.
Transfers and automatic Bash setup are enabled by default. An empty download
path selects the Windows Downloads folder. Reconnect after changing settings.

## Usage

After the shell prompt appears, drag files or folders from Explorer onto PuTTY.
The upload target is the directory reported by the last prompt. After `cd`,
wait for the next prompt before dropping files. Avoid dropping while a command
or full-screen program is running.

Download files and directories with:

```sh
cp reports.zip "$win"
cp "file with spaces.zip" "$win"
cp -r reports "$win"
cp -r reports/ "$win"
cp -r . "$win"
cp first.zip second.zip "$win"
cp -- -leading-dash.zip "$win"
```

The shell function handles `$win` with `-r`, `-R`, `-i` and `--`. The `-i`
option uses the required Windows confirmation, so an alias such as `cp='cp -i'`
also works. Other destinations use the system `cp`; `command cp` always bypasses
the function. Do not set `$win` to a real directory on the server.

Downloads require confirmation in Windows, with No selected by default. A
command can request up to 16 sources. The shell return code reports submission
of the request; the transfer result appears in Windows because transfers are
asynchronous.

Existing local files are not overwritten. Name conflicts get an unused suffix.
Downloads are staged in a `.putty-transfer-*` directory and renamed on success.
A failed or cancelled download can leave a partial staging directory; remove it
after the operation has closed.

The transfer window shows the current file, percentage, MiB transferred, speed
and ETA. For directories, progress describes the current file rather than the
whole tree. Cancel stops the transfer. Text details are shown after a failure.

## Bash setup

New interactive sessions prepare Bash through the SSH startup stream using
`/dev/fd/3`. The integration runs in shell memory without saving a helper or
editing `.bashrc` or `.profile`. Login profiles are read first, then `$win`, the
`cp` function and directory reporting are added. Existing prompt hooks are
preserved. Loading the integration again does not duplicate its hooks.

Automatic setup requires Bash as the user's `$SHELL`, an interactive PTY and no
custom remote command or subsystem. Other shells start normally with a notice
that automatic integration requires Bash. **Prepare Bash automatically** can be
disabled for restricted servers. The optional `contrib/putty-transfer.sh` helper
remains available for custom setups and Zsh; Zsh support has not been tested.

Transfers use a private SSH connection-sharing endpoint so PSCP can use the
current authenticated connection. This overrides ordinary sharing settings for
that GUI session. See [Duplicate Session](DUPLICATE-SESSION.md) for connection
lifetime and duplicate windows.

## Limits

- Directory reporting comes from the last prompt, not from screen text.
- Transfers use the original SSH user and host. They do not inherit `sudo`,
  `su` or an SSH connection opened inside the shell.
- `tmux` and `screen` may filter the control sequences. The helper does not
  configure passthrough for them.
- Paths support spaces and Unicode. The protocol allows at most 950 path bytes;
  paths unsuitable for Windows are rejected locally.
- Symbolic links cannot be download sources. Other advanced `cp` options are
  not supported for `$win`.
- Directories, including `.` and `..`, are resolved to physical absolute paths.
  The root directory `/` and paths containing control characters are rejected.
  An invalid directory report clears the previous upload target.

## Protocol

The helper sends `OSC 777` terminated by BEL or ST:

```text
ESC ] 777 ; putty-transfer;cwd;<hex absolute path> BEL
ESC ] 777 ; putty-transfer;download;<hex absolute path> BEL
```

Paths are UTF-8 bytes encoded as hexadecimal. The terminal forwards only complete,
bounded sequences without NUL bytes to the Windows frontend callback. Other
frontends ignore the protocol. The callback validates the SSH session, message
structure and paths. Remote output is untrusted, so downloads still require
local confirmation.
