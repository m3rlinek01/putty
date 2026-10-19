# Author: m3rlinek.
"""Check the actual portable ZIP without compiler DLLs or developer PATH.

Run on Windows: python test/portable_windows.py package.zip --readobj PATH
GUI checks only start isolated processes hidden and terminate those processes;
they do not type commands, connect to servers or interact with user windows.
"""
import argparse
import ctypes
import os
from pathlib import Path
import re
import subprocess
import tempfile
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument("archive", type=Path)
parser.add_argument("--readobj", required=True, type=Path)
args = parser.parse_args()
if os.name != "nt":
    raise SystemExit("This packaging check requires Windows")

programs = ("putty", "puttytel", "plink", "pscp", "psftp", "puttygen",
            "pageant", "pterm", "psocks")
system = Path(os.environ["SystemRoot"]) / "System32"
environment = os.environ.copy()
environment["PATH"] = str(system)
# Suppress loader-error dialogs in the child: failures must be test failures.
previous_mode = ctypes.windll.kernel32.SetErrorMode(0x8003)
try:
    with tempfile.TemporaryDirectory(prefix="putty portable ") as folder:
        directory = Path(folder)
        with zipfile.ZipFile(args.archive) as package:
            for member in package.infolist():
                target = (directory / member.filename).resolve()
                if not target.is_relative_to(directory.resolve()):
                    raise AssertionError("Archive entry escapes extraction directory")
            package.extractall(directory)
        assert not list(directory.glob("*.dll")), "Static package unexpectedly contains DLLs"
        for name in programs:
            exe = directory / (name + ".exe")
            result = subprocess.run([str(args.readobj), "--coff-imports", str(exe)],
                                    check=True, capture_output=True, text=True)
            imports = re.findall(r"^\s+Name: (\S+)", result.stdout, re.MULTILINE)
            assert imports, f"No PE imports inspected for {name}"
            for dll in imports:
                assert not dll.lower().startswith(("libc++", "libunwind", "libgcc",
                                                    "libstdc++", "libwinpthread")), (name, dll)
                assert dll.lower().startswith(("api-ms-", "ext-ms-")) or (system / dll).is_file(), (name, dll)
            print(f"{name}: system imports only")
        for name in ("plink", "pscp", "psftp"):
            result = subprocess.run([str(directory / (name + ".exe")), "-V"],
                                    cwd=directory, env=environment, capture_output=True,
                                    text=True, timeout=10, creationflags=subprocess.CREATE_NO_WINDOW)
            assert result.returncode == 0, (name, result.returncode, result.stderr)
            assert "0.85" in result.stdout + result.stderr, name
            print(f"{name}: system-only PATH startup PASS")
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        for name in ("putty", "puttytel", "puttygen"):
            process = subprocess.Popen([str(directory / (name + ".exe"))],
                                       cwd=directory, env=environment, startupinfo=startup,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                try:
                    status = process.wait(timeout=1.5)
                except subprocess.TimeoutExpired:
                    print(f"{name}: hidden GUI startup with system-only PATH PASS")
                else:
                    raise AssertionError(f"{name} exited during startup: {status:#x}")
            finally:
                if process.poll() is None:
                    process.terminate()
                process.wait(timeout=10)
finally:
    ctypes.windll.kernel32.SetErrorMode(previous_mode)
print("Portable ZIP checks passed.")
