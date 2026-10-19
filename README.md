# PuTTY for Windows

PuTTY 0.85 compiled as C++20 with LLVM-MinGW, with shell file transfers and
session duplication over an authenticated SSH connection.
Maintained by **m3rlinek**. This is an unofficial PuTTY build.

## Download

[Releases](https://github.com/m3rlinek01/putty/releases/latest) include installers
and portable ZIPs for x64 and x86. Use x64 on Windows 10/11 or x86 on Windows 10.
The x86 build was tested through WOW64 on Windows x64, not on a 32-bit OS.

The installer runs per user and adds Start menu shortcuts and an uninstaller.
For portable use, extract the ZIP and run `putty.exe`. Keep `pscp.exe` beside it.
Compiler runtimes are linked statically; no extra compiler DLLs are needed.
The executables are unsigned.

Packages contain PuTTY, PuTTYtel, Plink, PSCP, PSFTP, PuTTYgen, Pageant, Pterm
and Psocks.

## File transfers and sessions

Drag files or folders from Explorer onto PuTTY to upload them to the directory
reported by the last remote shell prompt. Wait for the prompt after `cd`.

To download to Windows:

```sh
cp reports.zip "$win"
cp -r reports "$win"
```

Downloads go to the Windows Downloads folder unless another folder is set in
**Connection > SSH > Files**. Each download request requires confirmation in
Windows. Transfers show a progress bar and can be cancelled.

Interactive Bash sessions are prepared automatically when connecting. No helper
installation or profile edits are needed on the server. The server must support
SFTP. Transfers use the original SSH user and host, including when the terminal
has entered `sudo`, `su` or another SSH session.

**Duplicate Session** opens a separate shell over the authenticated SSH
connection without another password prompt. Keep the original window open;
closing it disconnects its duplicates.

See [file transfers](docs/FILE-TRANSFER.md) and
[session duplication](docs/DUPLICATE-SESSION.md) for settings and limitations.

## Build

Use Windows x64, CMake 3.21+ and LLVM-MinGW UCRT. The tested compiler is Clang
22.1.8 from release `20260616`. Add the toolchain's `bin` directory to PATH.
The source still uses Clang extensions for C99 initializers; MSVC, GCC and
Unix/macOS builds have not been verified.

```powershell
.\build-cpp.ps1 -Architecture x64
.\build-cpp.ps1 -Architecture x86
```

Outputs go to `dist/` and `dist/x86/`. Build directories are `build/` and
`build-x86/`. See [setup](docs/GETTING-STARTED.md) and
[packaging](docs/PACKAGING.md) for dependencies and installer builds.

## Tests

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r test\requirements-port.txt
.\build-cpp.ps1 -Architecture x64 -Test
.\build-cpp.ps1 -Architecture x86 -Test
```

The full CTest run contains 13 suites, including 90 cryptographic tests and a
local SSH/SFTP integration test. Python is required for the cryptographic suite;
Paramiko is also required for SSH/SFTP. The integration server listens on
`127.0.0.1` and uses temporary keys and files.

Both architectures passed all 13 suites. Portable and installer checks cover
system-only PATH startup, package contents, installation, reinstall and
uninstallation. See [validation](docs/FILE-TRANSFER-VALIDATION.md) for scope and
untested cases.

## Source and license

Based on the [PuTTY 0.85 sources](https://www.chiark.greenend.org.uk/~sgtatham/putty/releases/0.85.html).
Archive metadata is in [UPSTREAM.json](UPSTREAM.json). The upstream README is
preserved as [README](README).

PuTTY's license is in [LICENCE](LICENCE). The statically linked LLVM runtime
license is in [THIRD-PARTY-LLVM-LICENSE.txt](THIRD-PARTY-LLVM-LICENSE.txt).

Developer notes: [development](docs/DEVELOPMENT.md),
[code map](docs/ARCHITECTURE.md), [planned work](docs/MODERNIZATION.md).
