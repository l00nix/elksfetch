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

## Version 0.1 draft

`elks-v0.1.png` is the QEMU framebuffer capture from the exact version 0.1
executable, with networking configured. Build identity and checks are in
`../releases/v0.1.md`. The attached HP image remains the preceding development
build's physical-device capture, as identified in the release notes.

## PPC 512 version 0.1 hardware captures

`ppc512-mono.png` and `ppc512-compact.png` show the exact 0.1 release executable
running on the physical Amstrad PPC 512 with PicoMEM. They use `--mono` and
`--plain --width 40` respectively, with `--host "Amstrad PPC 512"`.
The companion `.txt` files decode the actual character cells.

The existing FTP/telnet connection transferred the executable and captures.
For each image, the program rendered directly to `/dev/tty1`, then 4,000 bytes
were read from physical B8000h through `/dev/kmem`. The 128-character 8×8 ROM
font was freshly read from FFA6Eh. A renderer combined those bytes with the
CGA palette and doubled each scan line to produce a 640×400 PNG. It did not
synthesize output from a text transcript. BIOS data confirmed mode 3, 80 columns
and active page 0. These are logical display captures, not photographs of the
LCD; its tint/contrast and cursor/blink phase are not reconstructed.

See [test results](../PPC512-20261101.md) and
[hashes/capture metadata](../PPC512-20261101.json). Raw video cells, BIOS font
and BIOS data-area bytes are retained in the local hardware evidence directory.
