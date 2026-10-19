# Author: m3rlinek.
"""Install into an isolated directory, check ZIP parity, upgrade and uninstall.

Requires Windows; refuses to replace an existing registered installation.
"""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
import time
import winreg
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument("installer", type=Path)
parser.add_argument("archive", type=Path)
parser.add_argument("--architecture", choices=("x64", "x86"), required=True)
args = parser.parse_args()
key_path = (r"Software\Microsoft\Windows\CurrentVersion\Uninstall"
            + "\\PuTTY-m3rlinek-" + args.architecture + "_is1")
registry_view = winreg.KEY_WOW64_64KEY if args.architecture == "x64" else winreg.KEY_WOW64_32KEY


def registered():
    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, key_path, 0,
                            winreg.KEY_READ | registry_view):
            return True
    except FileNotFoundError:
        return False


assert not registered(), "An installation already exists; test refuses to replace it"
installer = args.installer.resolve()
environment = os.environ.copy()
environment["PATH"] = str(Path(os.environ["SystemRoot"]) / "System32")
with tempfile.TemporaryDirectory(prefix="putty installer ") as folder:
    target = Path(folder) / "installed"
    command = [str(installer), "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART",
               "/NOICONS", "/LANG=polish", f"/DIR={target}", "/TASKS="]
    try:
        for phase in ("install", "upgrade"):
            subprocess.run(command, check=True, timeout=120, env=environment)
            assert registered(), "Missing uninstall registration"
            with zipfile.ZipFile(args.archive) as archive:
                for member in archive.infolist():
                    assert not member.is_dir()
                    data = (target / member.filename).read_bytes()
                    assert hashlib.sha256(data).digest() == hashlib.sha256(archive.read(member)).digest(), member.filename
            for name in ("plink", "pscp", "psftp"):
                result = subprocess.run([str(target / (name + ".exe")), "-V"],
                                        capture_output=True, text=True, timeout=10,
                                        env=environment, creationflags=subprocess.CREATE_NO_WINDOW)
                assert result.returncode == 0 and "0.85" in result.stdout + result.stderr, name
            print(f"{args.architecture}: {phase}, file hashes and startup PASS")
    finally:
        uninstaller = target / "unins000.exe"
        if uninstaller.exists():
            subprocess.run([str(uninstaller), "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART"],
                           check=True, timeout=120)
    for attempt in range(50):
        if not target.exists() and not registered():
            break
        time.sleep(0.1)
    assert not target.exists(), "Installation directory remains after uninstall"
    assert not registered(), "Uninstall registration remains"
    print(f"{args.architecture}: uninstall PASS")
