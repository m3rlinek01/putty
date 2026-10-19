# Author: m3rlinek.
"""Execute the embedded SSH startup locally with Bash, without installing a helper.

Uses isolated HOME profiles and checks generated code matches the embedded bytes.
Run after CMake configuration: python test/transfer_bootstrap.py --bash PATH
"""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

parser = argparse.ArgumentParser()
parser.add_argument("--bash", default="bash")
args, remaining = parser.parse_known_args()
root = Path(__file__).resolve().parents[1]
startup = (root / "build/CMakeFiles/putty-transfer-bootstrap.sh").read_text(encoding="utf-8")

class Bootstrap(unittest.TestCase):
    def run_bootstrap(self, profile="", commands="", shell="/bin/bash"):
        with tempfile.TemporaryDirectory(prefix="putty startup ") as tmp:
            home = Path(tmp)
            (home / ".bash_profile").write_text(profile, encoding="utf-8")
            (home / "file.zip").write_bytes(b"file")
            before = set(home.iterdir())
            env = dict(os.environ, HOME=home.as_posix(), SHELL=shell)
            proc = subprocess.run([args.bash, "--noprofile", "--norc", "-c", startup],
                                  cwd=home, env=env, input=commands.encode(),
                                  capture_output=True, timeout=15)
            # Interactive Bash may save its ordinary history on exit. The
            # bridge must not create an rc/helper or edit the existing profile.
            self.assertEqual(set(home.iterdir()) - {home / ".bash_history"}, before,
                             "Bootstrap installed a server file")
            self.assertEqual((home / ".bash_profile").read_text(encoding="utf-8"), profile)
            return proc

    def test_embedded_bytes(self):
        header = (root / "build/CMakeFiles/file-transfer-bootstrap.h").read_text(encoding="utf-8")
        self.assertIn('R"PUTTYSH(' + startup + ')PUTTYSH"', header)
        self.assertNotIn("@PUTTY_TRANSFER_HELPER@", startup)

    def test_ready_without_helper(self):
        result = self.run_bootstrap(commands='printf "READY:%s\\n" "$win"; type cp; exit\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"READY:__PUTTY_WINDOWS_DOWNLOAD__", result.stdout)
        self.assertIn(b"cp is a function", result.stdout)
        self.assertIn(b"\x1b]777;putty-transfer;cwd;", result.stdout)

    def test_existing_login_profile_and_prompt(self):
        result = self.run_bootstrap(profile='export MY_PROFILE_VALUE=preserved\nPROMPT_COMMAND="printf PROFILE_HOOK"\n',
            commands='printf "VALUE:%s\\n" "$MY_PROFILE_VALUE"; cp file.zip "$win"; exit\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"VALUE:preserved", result.stdout)
        self.assertIn(b"PROFILE_HOOK", result.stdout)
        self.assertIn(b"\x1b]777;putty-transfer;download;", result.stdout)

    def test_normal_cp_and_no_leaked_startup_fd(self):
        result = self.run_bootstrap(commands='cp file.zip /dev/null; printf "CP:%s\\n" "$?"; '
            'if : <&3 2>/dev/null; then echo FD_LEAK; else echo FD_CLOSED; fi; exit\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"CP:0", result.stdout)
        self.assertIn(b"FD_CLOSED", result.stdout)
        self.assertNotIn(b"FD_LEAK", result.stdout)

    def test_unsupported_shell_falls_back(self):
        result = self.run_bootstrap(commands='echo FALLBACK; exit\n', shell="/bin/sh")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"FALLBACK", result.stdout)
        self.assertIn(b"requires Bash", result.stderr)

if __name__ == "__main__":
    unittest.main(argv=[__file__] + remaining)
