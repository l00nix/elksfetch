#!/usr/bin/env python3
"""Host-side tests for public CLI behavior; no ELKS-runtime claims."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class RenderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.exe = str(Path(cls.tmp.name) / 'fetch-fixture')
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c89', '-pedantic',
                        '-Wall', '-Wextra', '-Werror', '-Isrc', 'src/main.c',
                        'src/render.c', 'tests/fixture.c', '-o', cls.exe], cwd=ROOT, check=True)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def run_fetch(self, *args):
        return subprocess.run([self.exe, *args], capture_output=True, text=True,
                              env={**os.environ, 'TERM': 'vt100'})

    def test_command_name_and_version(self):
        self.assertEqual(self.run_fetch('--version').stdout, 'elksftch 0.1.1\n')
        self.assertIn('Usage: elksftch [options]', self.run_fetch('--help').stdout)
        self.assertTrue(self.run_fetch('--unknown').stderr.startswith('elksftch:'))

    def test_every_supported_width_and_logo(self):
        for width in range(20, 241):
            for logo in ('small', 'wide', 'none'):
                p = self.run_fetch('--width', str(width), '--logo', logo)
                self.assertEqual(p.returncode, 0)
                self.assertNotIn('\x1b', p.stdout)
                self.assertLess(max(map(len, p.stdout.splitlines())), width)

    def test_untrusted_text_is_bounded_and_sanitized(self):
        p = self.run_fetch('--plain', '--host', 'x\x1b[2J\n\t' + 'A' * 500)
        self.assertEqual(p.returncode, 0)
        self.assertNotIn('\x1b', p.stdout)
        self.assertIn('Model (set): x?[2J??', p.stdout)
        self.assertLess(max(map(len, p.stdout.splitlines())), 80)

    def test_all_fields_fit_capacity(self):
        p = self.run_fetch('--plain', '--host', 'Full fixture')
        self.assertEqual(len(p.stdout.splitlines()), 16)
        self.assertIn('Gateway: 192.168.100.1', p.stdout)
        self.assertIn('TTY: /dev/tty1', p.stdout)

    def test_invalid_options(self):
        for args in [('--width',), ('--host',), ('--host', ''), ('--unknown',),
                     ('--width', '0'), ('--width', '-1'), ('--width', '241'),
                     ('--width', '99999999999999999'), ('--width', '80x'),
                     ('--logo', 'bad'), ('--color', 'bad')]:
            self.assertEqual(self.run_fetch(*args).returncode, 2, args)

    def test_colour_and_plain(self):
        p = self.run_fetch('--color', 'always')
        self.assertIn('\x1b[1;33m', p.stdout)
        visible = re.sub(r'\x1b\[[0-9;]*m', '', p.stdout)
        self.assertLess(max(map(len, visible.splitlines())), 80)
        self.assertNotIn('\x1b', self.run_fetch('--plain').stdout)
        self.assertNotIn('EEEEEE', self.run_fetch('--plain').stdout)

    def test_wide_art_fallback(self):
        self.assertNotIn('#######################', self.run_fetch('--logo', 'wide').stdout)
        self.assertIn('#######################', self.run_fetch('--logo', 'wide', '--width', '132').stdout)

if __name__ == '__main__':
    unittest.main()
