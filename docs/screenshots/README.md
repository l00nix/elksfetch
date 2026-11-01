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

## HP 200LX DS hardware capture

`hp200lx-ds.bmp` is the original ELXcap 0.1 capture from the physical HP 200LX
DS on 2026-11-01. The 16,062-byte file was downloaded over serial and its MD5
matched the value on the HP. `hp200lx-ds.png` is a lossless `sips` conversion.
No image content was composited, drawn over or generated. ELXcap reconstructs
text-mode captures from the active logical text page and font; LCD zoom/pan,
cursor and hardware inversion are outside its capture scope.
See `../HP200LX-DS-20261101.md` and its JSON manifest for test details/hashes.

## Networking update

`elks-network.png` is a new QEMU framebuffer capture with NE2000 networking
configured. `network-serial.txt` records the disconnected → configured →
disconnected checks. The original three QEMU captures above remain historical.

`hp200lx-network.bmp` is a second ELXcap capture of the physical HP 200LX DS,
now displaying `Network: disconnected`. Its MD5, verified on both ends of
the serial transfer, is `450f338977eceab23a590bc17aca532a`.
`hp200lx-network.png` is its lossless PNG conversion and the current README
hero. The same ELXcap limitations described above apply.
See `../NETWORKING.md` for the revised executable and test details.
