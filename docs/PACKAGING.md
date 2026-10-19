# Windows packages

Each architecture has an EXE installer and a portable ZIP containing all nine
programs, transfer instructions and licenses. Compiler runtimes are linked
statically. Targets require Windows 10/11 x64 or Windows 10 x86.

## Build

Add LLVM-MinGW UCRT, CMake and Inno Setup 7.1.0 (`ISCC.exe`) to PATH.
Download Inno Setup from its [official site](https://jrsoftware.org/isdl.php).

```powershell
.\build-cpp.ps1 -Architecture x64 -Test
.\build-cpp.ps1 -Architecture x86 -Test
.\package-windows.ps1 -Architecture x64
.\package-windows.ps1 -Architecture x86
```

Optional arguments are `-InstallerCompiler C:\Tools\InnoSetup\ISCC.exe`,
`-PackageDirectory`, `-OutputDirectory` and `-Version 0.85-cpp.2`.
The script checks each executable's PE architecture before packaging and uses
an explicit file list. It does not include other architecture subdirectories.

Binaries are in `dist/` and `dist/x86/`; packages are written to `out/`.

## Installer behavior

The installer supports Polish and English and runs without administrator
privileges. It installs to `%LOCALAPPDATA%\Programs\PuTTY-m3rlinek-x64` or
`PuTTY-m3rlinek-x86`, with Start menu shortcuts and an optional desktop shortcut.
The variants use separate installation IDs and can coexist.

Installation does not change PATH or saved PuTTY sessions. Uninstallation
removes installed files, shortcuts and the uninstall entry, leaving session
settings in place. Installers and application executables are unsigned.

## Package checks

```powershell
python test/portable_windows.py out/PuTTY-0.85-cpp.2-windows-x86-portable.zip --architecture x86 --readobj C:\Tools\llvm-mingw\bin\llvm-readobj.exe
python test/installer_windows.py out/PuTTY-0.85-cpp.2-windows-x86-setup.exe out/PuTTY-0.85-cpp.2-windows-x86-portable.zip --architecture x86
```

Repeat with `x64`. The portable check validates PE headers, system imports and
startup without the toolchain in PATH. The installer check uses a temporary
directory to test silent installation, file hashes against the ZIP, reinstall
and uninstall. It refuses to replace an existing registered installation.

The x86 package was tested through WOW64 on Windows x64. A separate Windows 10
32-bit installation has not been tested.
