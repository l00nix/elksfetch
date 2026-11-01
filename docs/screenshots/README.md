# Screenshot provenance

Captured 2026-11-01 by `tests/qemu_smoke.py` from the actual ELKS console in
QEMU. The binary came through the candidate `buildext.sh` and selected
`ExtApplications` install path; its SHA-256 is recorded in `../VALIDATION.md`.

- `elks-colour.png`: `elksfetch --color always`
- `elks-mono.png`: `elksfetch --mono`
- `elks-compact.png`: `elksfetch --plain --width 40`
- `serial.txt`: guest test-session transcript, including the missing-device test.

The test runner connected stdin/stdout to `/dev/tty1` for screenshots. QEMU's
`screendump` supplied PPM framebuffer captures; macOS `sips -s format png`
converted them to PNG. No content was painted over, composited or invented.
The console is 80 columns; the compact capture explicitly limits the program
output to 40 columns. These are emulator captures, not real-hardware photos.
