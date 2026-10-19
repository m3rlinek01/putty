# Author: m3rlinek.
"""Focused shell-helper contract checks; run with Python and Bash on PATH.

Pass --bash PATH when Windows PATH selects the WSL launcher instead of Git Bash.
These checks do not contact an SSH server or start file transfers.
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
HELPER = Path(__file__).resolve().parents[1] / "contrib" / "putty-transfer.sh"


class ShellTransfer(unittest.TestCase):
    def setUp(self):
        self.scratch = tempfile.TemporaryDirectory(prefix="putty shell ")
        self.addCleanup(self.scratch.cleanup)
        self.cwd = Path(self.scratch.name)
        (self.cwd / "file with spaces.zip").write_bytes(b"contents")
        (self.cwd / "zażółć.zip").write_bytes(b"unicode")
        (self.cwd / "folder").mkdir()

    def run_shell(self, code):
        environment = dict(os.environ, PUTTY_TRANSFER_HELPER=HELPER.as_posix())
        return subprocess.run(
            [args.bash, "--noprofile", "--norc", "-c",
             'source "$PUTTY_TRANSFER_HELPER" >/dev/null; ' + code],
            cwd=self.cwd, env=environment, capture_output=True,
        )

    def requests(self, result):
        self.assertEqual(result.returncode, 0, result.stderr)
        requests = []
        for sequence in result.stdout.split(b"\x07"):
            if not sequence:
                continue
            self.assertTrue(sequence.startswith(b"\x1b]777;putty-transfer;"), sequence)
            verb, hexadecimal = sequence[len(b"\x1b]777;putty-transfer;"):].split(b";")
            requests.append((verb, bytes.fromhex(hexadecimal.decode()).decode("utf-8")))
        return requests

    def test_spaces_unicode_and_multiple_sources(self):
        requests = self.requests(self.run_shell('cp "file with spaces.zip" "zażółć.zip" "$win"'))
        self.assertEqual([verb for verb, _ in requests], [b"download", b"download"])
        self.assertTrue(requests[0][1].endswith("/file with spaces.zip"))
        self.assertTrue(requests[1][1].endswith("/zażółć.zip"))

    def test_recursive_and_directory_guard(self):
        result = self.run_shell('cp folder "$win"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")
        requests = self.requests(self.run_shell('cp -r folder "$win"'))
        self.assertTrue(requests[0][1].endswith("/folder"))

    def test_validate_all_sources_before_submission(self):
        result = self.run_shell('cp "file with spaces.zip" missing.zip "$win"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")

    def test_ordinary_cp_keeps_its_behavior(self):
        result = self.run_shell('cp "file with spaces.zip" copied.zip')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, b"")
        self.assertEqual((self.cwd / "copied.zip").read_bytes(), b"contents")

    def test_options_and_leading_hyphen(self):
        (self.cwd / "-file.zip").write_bytes(b"hyphen")
        requests = self.requests(self.run_shell('cp -- -file.zip "$win"'))
        self.assertTrue(requests[0][1].endswith("/-file.zip"))
        result = self.run_shell('cp -a folder "$win"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")

    def test_prompt_hooks_preserved_and_not_duplicated(self):
        result = self.run_shell(
            'unset _PUTTY_TRANSFER_HOOK_INSTALLED; '
            'PROMPT_COMMAND=("printf old-hook"); '
            'source "$PUTTY_TRANSFER_HELPER" >/dev/null; '
            'source "$PUTTY_TRANSFER_HELPER" >/dev/null; '
            'printf "%s\\n" "${PROMPT_COMMAND[@]}"'
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, b"printf old-hook\n_putty_transfer_cwd\n")

    def test_interactive_cp_alias_keeps_working(self):
        result = self.run_shell(
            "shopt -s expand_aliases; alias cp='cp -i'; "
            'source "$PUTTY_TRANSFER_HELPER" >/dev/null\n'
            'cp "file with spaces.zip" "$win"'
        )
        requests = self.requests(result)
        self.assertTrue(requests[0][1].endswith("/file with spaces.zip"))

    def test_cwd_changes(self):
        requests = self.requests(self.run_shell('cd folder; _putty_transfer_cwd'))
        self.assertEqual(requests[0][0], b"cwd")
        self.assertTrue(requests[0][1].endswith("/folder"))

    def test_source_count_bound_before_submission(self):
        sources = '"file with spaces.zip" ' * 16
        self.assertEqual(len(self.requests(self.run_shell('cp ' + sources + '"$win"'))), 16)
        result = self.run_shell('cp ' + sources + '"file with spaces.zip" "$win"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")

    def test_path_bound_counts_utf8_bytes(self):
        self.assertEqual(len(self.requests(self.run_shell('_putty_transfer_emit cwd "' + 'é' * 475 + '"'))), 1)
        result = self.run_shell('_putty_transfer_emit cwd "' + 'é' * 476 + '"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")

    def test_directory_trailing_slash_and_dot_normalized(self):
        requests = self.requests(self.run_shell('cp -r folder/ "$win"; cp -r . "$win"'))
        self.assertEqual(len(requests), 2)
        self.assertTrue(requests[0][1].endswith("/folder"))
        self.assertEqual(requests[1][1].rsplit("/", 1)[1], self.cwd.name)

    def test_parent_directory_normalized_without_changing_cwd(self):
        requests = self.requests(self.run_shell('cd folder; cp -r .. "$win"; _putty_transfer_cwd'))
        self.assertEqual(requests[0][1].rsplit("/", 1)[1], self.cwd.name)
        self.assertTrue(requests[1][1].endswith("/folder"))

    def test_control_path_rejected_before_any_submission(self):
        result = self.run_shell('cp "file with spaces.zip" $\'bad\\nname\' "$win"')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"control characters", result.stderr)

    def test_invalid_cwd_clears_previous_destination(self):
        for invalid in ['bad\npath', 'x' * 951]:
            result = self.run_shell('PWD="' + invalid + '"; _putty_transfer_cwd')
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(result.stdout, b"\x1b]777;putty-transfer;cwd;\x07")


if __name__ == "__main__":
    unittest.main(argv=[__file__, *remaining])
