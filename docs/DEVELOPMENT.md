# Development

The C++ port and Windows extensions are maintained by **m3rlinek**. Upstream
PuTTY authorship and licensing remain in `LICENCE` and `UPSTREAM.json`.

Build on Windows x64 with LLVM-MinGW UCRT and CMake 3.21+. Both x64 and x86
outputs use C++20 with Clang extensions for inherited C99 initializers. See
[setup](GETTING-STARTED.md) and [toolchain](TOOLCHAIN.md) for the tested versions.
MSVC, GCC and Unix/macOS builds have not been verified.

```powershell
.\build-cpp.ps1 -Architecture x64 -Test
.\build-cpp.ps1 -Architecture x86 -Test
```

Compiler runtimes are linked statically. The build script rejects executables
that import compiler runtime DLLs. Build outputs and caches are ignored by Git.
Assertions remain enabled in Release; strict aliasing is disabled.

The full test run needs Python and Paramiko from `test/requirements-port.txt`.
It contains 13 CTest suites, including crypto and local SSH/SFTP checks.
`test/transfer_shell.py` and `test/transfer_bootstrap.py` test Bash integration.
`test/portable_windows.py` checks extracted packages without the toolchain in
PATH; `test/installer_windows.py` checks install, reinstall and uninstall.
GUI startup tests do not cover interactive behavior.

Changes to protocols, key formats, secret-memory clearing and coroutine state
need regression tests for the affected behavior. Keep resource ownership clear
when changing callbacks or error paths.

See the [code map](ARCHITECTURE.md), [planned work](MODERNIZATION.md) and
[packaging instructions](PACKAGING.md).
