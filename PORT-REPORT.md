# Port status

PuTTY 0.85 is compiled as C++20 with LLVM-MinGW Clang extensions. The original
procedural structure, algorithms and license are retained. Upstream archive
metadata is in [UPSTREAM.json](UPSTREAM.json). The port and Windows extensions
are maintained by **m3rlinek**.

All nine Windows applications build for x64 and x86. Compiler runtimes are
linked statically; remaining imports are Windows system libraries. Distribution
packages include the LLVM runtime license. MSVC, GCC and Unix/macOS builds
have not been verified.

Windows additions include Explorer uploads, downloads through `cp file "$win"`,
automatic in-memory Bash integration, native transfer progress and Duplicate
Session over an authenticated connection.

Both architectures passed 13 CTest suites, including 90 cryptographic tests
and local SSH/SFTP integration. Bash helper and startup checks passed separately.
Portable checks cover imports and startup without the toolchain in PATH.
Installer checks cover installation, file hashes, reinstall and uninstallation.
The x86 build was tested through WOW64 rather than a separate 32-bit OS.

See [transfer validation](docs/FILE-TRANSFER-VALIDATION.md) for test coverage and
untested cases. The executables and installers are unsigned.
