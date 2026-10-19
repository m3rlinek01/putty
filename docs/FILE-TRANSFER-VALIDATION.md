# Transfer validation

Validated on Windows x64 with LLVM-MinGW Clang 22.1.8 (`20260616`), CMake 4.4.4,
Python 3.13.2 and Paramiko 5.0.0. Both x64 and x86 builds passed 13/13 CTest suites
for release `v0.85-cpp.2` on 2026-10-19. The x86 tests ran through WOW64.

## Coverage

| Check | Coverage |
|---|---|
| `file_transfer` | 158 checks of paths, reserved Windows names, UTF-8, argument quoting and progress records |
| `terminal` | OSC 777, BEL/ST termination, fragmented input, NUL, overflow recovery and absent callbacks |
| `conf` | Configuration copying and serialization, nested duplicate IDs, disabled options and ineligible sessions |
| `crypto` | 90 upstream cryptographic tests |
| `local_ssh_sftp` | 13 checks, including host-key rejection, recursive Unicode transfer, literal brackets, junction rejection and shared shell channels |
| `transfer_shell.py` | 14 Bash helper tests |
| `transfer_bootstrap.py` | 5 startup tests using Git Bash |
| `portable_windows.py` | PE architecture, imports of nine programs, CLI startup and hidden GUI startup with system-only PATH |
| `installer_windows.py` | Silent install, hashes against portable ZIP, CLI startup, reinstall and uninstall for both architectures |

The SSH/SFTP tests check that additional shells do not create another TCP
connection or authentication attempt. Transfer tests also check that a missing
or incorrect private endpoint does not fall back to a new connection.

Bootstrap checks cover embedded script data, `$win` and `cp`, existing profiles
and prompt hooks, ordinary `cp`, closing fd 3 and fallback for non-Bash shells.
The helper is not installed on the server.

Progress checks include fragmented records, overflow, UTF-8, empty files and
buffer limits. SFTP integration also checks records emitted by PSCP.

Portable tests extract the actual archive to a temporary directory, reject
compiler DLL dependencies and start Plink, PSCP and PSFTP with PATH restricted
to Windows System32. PuTTY, PuTTYtel and PuTTYgen are started hidden and stopped
by the test. Installer tests refuse to replace an existing installation.

## Logs and limits

CTest logs are local to `build/Testing/Temporary/LastTest.log` and
`build-x86/Testing/Temporary/LastTest.log`. The integration report is
`validation/integration.json`. These generated files are ignored by Git.

Automated checks do not cover Explorer drag-and-drop interaction, the Duplicate
Session menu or visual layout during a transfer. Zsh, a separate 32-bit Windows
installation, physical serial ports, Kerberos/GSSAPI and all remote SSH server
configurations have not been tested. Process startup checks are not interactive
GUI tests.
