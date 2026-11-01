#!/usr/bin/env python3
"""Run the actual ELKS binary in a disposable FAT image; retain logs and PPMs.
Requires Python 3, QEMU and mtools. The supplied image is never changed.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time

p = argparse.ArgumentParser()
p.add_argument('--image', type=Path, required=True, help='Bootable ELKS unpartitioned FAT HD image')
p.add_argument('--binary', type=Path, required=True)
p.add_argument('--out', type=Path, required=True)
p.add_argument('--chroot-helper', type=Path, help='Optional compiled tests/without_kmem.c')
a = p.parse_args()
a.out.mkdir(parents=True, exist_ok=True)
a.out = a.out.resolve()
with tempfile.TemporaryDirectory(prefix='fetch-qemu-') as tmp:
    temp = Path(tmp)
    disk = temp / 'disk.img'
    shutil.copyfile(a.image, disk)
    boot = subprocess.check_output(['mtype', '-i', str(disk), '::/bootopts']).decode()
    boot += '\nconsole=ttyS0,19200 3\n'
    assert len(boot.encode()) <= 1023, 'bootopts exceeds ELKS limit'
    (temp / 'bootopts').write_text(boot)
    subprocess.run(['mcopy', '-o', '-i', str(disk), str(temp / 'bootopts'), '::/bootopts'], check=True)
    subprocess.run(['mcopy', '-o', '-i', str(disk), str(a.binary), '::/bin/elksfetch'], check=True)
    if a.chroot_helper:
        subprocess.run(['mcopy', '-o', '-i', str(disk), str(a.chroot_helper), '::/bin/fetchroot'], check=True)
    serial, qmp = temp / 'serial', temp / 'qmp'
    log = (a.out / 'qemu.log').open('w')
    proc = subprocess.Popen(['qemu-system-i386', '-machine', 'pc', '-cpu', '486',
        '-accel', 'tcg,one-insn-per-tb=on', '-m', '16M', '-display', 'none',
        '-monitor', 'none', '-serial', f'unix:{serial},server=on,wait=off',
        '-qmp', f'unix:{qmp},server=on,wait=off',
        '-netdev', 'user,id=fetch', '-device', 'ne2k_isa,irq=12,netdev=fetch',
        '-drive', f'file={disk},format=raw,if=ide', '-boot', 'c'], stdout=log, stderr=log)
    transcript = bytearray()
    try:
        deadline = time.monotonic() + 10
        while not serial.exists() and time.monotonic() < deadline:
            time.sleep(.1)
        s = socket.socket(socket.AF_UNIX)
        s.connect(str(serial)); s.settimeout(.2)
        def receive_until(needle, timeout=20):
            data = bytearray(); end = time.monotonic() + timeout
            while time.monotonic() < end:
                try:
                    chunk = s.recv(8192)
                    if not chunk: raise RuntimeError('Serial connection closed')
                    data.extend(chunk); transcript.extend(chunk)
                except socket.timeout:
                    pass
                if needle in data:
                    return bytes(data)
            raise RuntimeError('Timed out waiting for ' + repr(needle) + '\n' + data.decode('latin1'))
        receive_until(b'login:', 45)
        s.sendall(b'root\r')
        receive_until(b'# ')
        def command(text):
            s.sendall(text.encode() + b'\r')
            return receive_until(b'# ')
        command('uname -a')
        plain = command('elksfetch --plain')
        assert b'OS: ELKS' in plain and b'Main RAM:' in plain and b'Uptime:' in plain
        assert b'unavailable' not in plain
        assert b'Network: disconnected' in plain and b'IPv4:' not in plain
        time.sleep(2)
        later = command('elksfetch --plain')
        assert plain.split(b'Uptime: ')[1].split(b'\r')[0] != later.split(b'Uptime: ')[1].split(b'\r')[0]
        assert b'\x1b' not in plain
        command('meminfo -m')
        command('ps -u')
        command('elksfetch --width 0; echo INVALID=$?')
        command('elksfetch --host "QEMU test guest" --compact --width 40')
        redirected = command('elksfetch > /tmp/fetch.txt; cat /tmp/fetch.txt')
        assert b'\x1b' not in redirected
        shortname = command('cp /bin/elksfetch /tmp/elkfetch; /tmp/elkfetch --plain')
        assert b'OS: ELKS' in shortname and b'Uptime:' in shortname
        q = socket.socket(socket.AF_UNIX); q.connect(str(qmp)); f = q.makefile('rwb')
        f.readline()
        def qcommand(name, args=None):
            f.write((json.dumps({'execute': name, 'arguments': args or {}}) + '\n').encode()); f.flush()
            while True:
                result = json.loads(f.readline())
                if 'error' in result: raise RuntimeError(result)
                if 'return' in result: return result['return']
        qcommand('qmp_capabilities')
        for name, options in [('elks-colour', '--color always'), ('elks-mono', '--mono'),
                              ('elks-compact', '--plain --width 40')]:
            command("echo -e '\\033[2J\\033[H' > /dev/tty1")
            command('elksfetch ' + options + ' < /dev/tty1 > /dev/tty1')
            time.sleep(.3)
            qcommand('screendump', {'filename': str(a.out / (name + '.ppm'))})
        # Only the disposable guest is configured. No host port forwarding.
        command('ktcp -b -p ne0 10.0.2.15 10.0.2.2 255.255.255.0')
        connected = command('elksfetch --plain')
        assert b'Network: connected (configured)' in connected
        assert b'IPv4: 10.0.2.15' in connected
        assert b'Netmask: 255.255.255.0' in connected
        assert b'Gateway: 10.0.2.2' in connected
        command("echo -e '\\033[2J\\033[H' > /dev/tty1")
        command('elksfetch --mono < /dev/tty1 > /dev/tty1')
        time.sleep(.3)
        qcommand('screendump', {'filename': str(a.out / 'elks-network.ppm')})
        command('net stop')
        stopped = command('elksfetch --plain')
        assert b'Network: disconnected' in stopped and b'IPv4:' not in stopped
        if a.chroot_helper:
            command('mkdir /tmp/froot')
            command('cp /bin/elksfetch /tmp/froot/fetch')
            missing = command('fetchroot; echo FALLBACK=$?')
            assert b'unavailable (/dev/kmem)' in missing
            assert b'FALLBACK=0' in missing
        command('sync')
        qcommand('quit'); proc.wait(timeout=10)
        assert b'INVALID=2' in transcript
        assert b'panic' not in transcript.lower()
        print('ELKS QEMU smoke test passed; screenshots and serial log:', a.out)
    finally:
        (a.out / 'serial.txt').write_bytes(transcript)
        if proc.poll() is None:
            proc.terminate(); proc.wait(timeout=10)
        log.close()
