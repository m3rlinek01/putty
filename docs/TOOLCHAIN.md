# Tested toolchain

| Component | Version |
|---|---|
| Build host | Windows x64 |
| Targets | Windows x64 and x86 |
| Language | C++20 with Clang extensions for C99 initializers |
| Compiler | LLVM-MinGW UCRT, Clang 22.1.8, release `20260616` |
| Generator | MinGW Makefiles / `mingw32-make` |
| CMake | 4.4.4; minimum required: 3.21 |
| Python | 3.13.2 |
| Paramiko | 5.0.0 |
| Installer compiler | Inno Setup 7.1.0 |

Toolchain release: [LLVM-MinGW 20260616](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260616).
The `llvm-mingw-20260616-ucrt-x86_64.zip` archive has SHA-256:

```text
b9b68a4d276e16fa25802aaba458e4638f64b3884c290aaccdc2d87083b6ca35
```

Check the downloaded archive with:

```powershell
Get-FileHash .\llvm-mingw-20260616-ucrt-x86_64.zip -Algorithm SHA256
```

This package includes `x86_64-w64-mingw32-clang++` and
`i686-w64-mingw32-clang++`. It is not an MSVC or `clang-cl` toolchain.
Strict ISO C++20, MSVC, GCC and Unix/macOS builds have not been verified.

The cryptographic test driver uses Python's standard library. Paramiko is needed
for the temporary SSH/SFTP integration server. Tests use local keys and files,
not credentials for an external server.
