#!/usr/bin/env python3
"""Test collector responses; native socket ABI is separately tested in QEMU."""
import os
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    exe = str(Path(tmp) / 'network-test')
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c89', '-pedantic',
                    '-Wall', '-Wextra', '-Werror', '-Isrc',
                    'tests/network_fixture.c', '-o', exe], cwd=root, check=True)
    subprocess.run([exe], check=True)
