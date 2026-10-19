# Build setup

Use Windows x64 to build both target architectures. For prebuilt installers and
portable packages, use [Releases](https://github.com/m3rlinek01/putty/releases/latest).

Install Git, GitHub CLI, CMake and Python if needed:

```powershell
winget install --exact --id Git.Git
winget install --exact --id GitHub.cli
winget install --exact --id Kitware.CMake
winget install --exact --id Python.Python.3.13
```

Open a new PowerShell after installation and check `git --version`,
`gh --version`, `cmake --version` and `py -3.13 --version`.

## Compiler

Download and extract the tested
[LLVM-MinGW UCRT package](https://github.com/mstorsjo/llvm-mingw/releases/download/20260616/llvm-mingw-20260616-ucrt-x86_64.zip).
See [TOOLCHAIN.md](TOOLCHAIN.md) for its checksum. Use LLVM-MinGW rather than
Visual Studio's `clang-cl`.

Add the extracted `bin` directory to PATH, adjusting the example path:

```powershell
$env:PATH = 'C:\Tools\llvm-mingw-20260616-ucrt-x86_64\bin;' + $env:PATH
x86_64-w64-mingw32-clang++ --version
i686-w64-mingw32-clang++ --version
mingw32-make --version
```

Both target compilers are included in the same package.

## Build and test

Authenticate with an account that has repository access. Skip `gh auth login`
if already authenticated.

```powershell
gh auth login
gh repo clone m3rlinek01/putty
Set-Location putty
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r test\requirements-port.txt
.\build-cpp.ps1 -Architecture x64 -Test
.\build-cpp.ps1 -Architecture x86 -Test
```

The script selects the project's `.venv\Scripts\python.exe`. To use another
interpreter, pass `-PythonExecutable C:\Path\python.exe`.

A full run has 13 CTest suites per architecture. The `crypto` suite contains
90 tests; `local_ssh_sftp` runs 13 checks against a temporary local server.
No external SSH credentials are needed.

Run `dist\putty.exe` for x64 or `dist\x86\putty.exe` for x86. Build caches are
in `build/` and `build-x86/`. See [PACKAGING.md](PACKAGING.md) to make installers
and portable ZIPs.

## Troubleshooting

- **11 tests:** CMake did not find Python.
- **12 tests:** the selected Python lacks Paramiko. Install the requirements
  with that interpreter and run the build script again.
- **Wrong compiler:** check `Get-Command x86_64-w64-mingw32-clang++, i686-w64-mingw32-clang++, mingw32-make`.
  Put the LLVM-MinGW `bin` directory first in PATH.
- **CMake errors after moving the repository:** configure a fresh build
  directory. Caches contain absolute paths and cannot be moved between machines.
- **Missing `pterm.exe`:** check ConPTY detection and the Windows toolchain.
  The tested configuration builds Pterm.
