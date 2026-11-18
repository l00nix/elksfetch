> ⚠️ I’m developing this project with AI assistance out of personal interest. It may not fully align with the ELKS maintainer team’s architectural or development guidelines. Please use it at your own discretion. ⚠️

# ELKSfetch

A small native system-information utility for **ELKS on 8086-class computers**.
An ASCII ELKS wordmark, a handful of useful facts, and no Bash, Python or modern
Linux filesystem dependencies on the target.

ELKSfetch is inspired by [Neofetch](https://github.com/dylanaraps/neofetch),
written as a new native C program for [ELKS](https://github.com/ghaerr/elks).

Intended hardware includes the Amstrad PPC 512 and HP 200LX. Initial
real-hardware smoke tests pass on both an HP 200LX DS and an Amstrad PPC 512.

The source repository is **public**. The current compatibility update is
**0.1.1**, with the eight-character command **`elksftch`** on both FAT and MINIX.
The project, GitHub repository and EPT package remain named **ELKSfetch** /
`elksfetch`. Version 0.1 artifacts and screenshots retain their original names.
See [0.1.1 release notes](docs/releases/v0.1.1.md) and the historical
[0.1 release notes](docs/releases/v0.1.md).

The native IA16 build and ELKS/QEMU runtime checks pass. Version 0.1 also
passes the [PPC 512 hardware smoke test](docs/PPC512-20261101.md).
The preceding development build passed an
[HP 200LX DS hardware smoke test](docs/HP200LX-DS-20261101.md).

DOSfetch is planned, not implemented. The upstream integration patch is a
candidate and has not been submitted.

![ELKSfetch on a real HP 200LX DS, captured with ELXcap](docs/screenshots/hp200lx-network.png)

*HP 200LX DS running the development build with network status, captured
with ELXcap 0.1 on 2026-11-01. The model label is explicitly configured.*

The original [640×200 BMP](docs/screenshots/hp200lx-network.bmp) is retained;
the PNG is a lossless conversion. ELXcap reconstructs the logical text page;
LCD zoom, hardware inversion and cursor appearance are not reproduced.

## What it shows

- ELKS release, hostname and kernel platform string from `uname()`.
- Current user, uptime and memory usage through ELKS interfaces.
- Main and XMS memory pools, with used/free values in KiB (`K`). These are
  kernel-managed pools, **not total installed physical RAM**. Usage includes
  ELKSfetch itself. XMS is shown when the kernel reports a nonzero pool;
  this does not imply XMS-backed application allocation is enabled.
- Live network state and IPv4 address, plus netmask/gateway on newer `ktcp`.
- Shell and terminal environment values, explicitly labelled as such.
- TTY where available, and an optional machine label explicitly marked `set`.

Missing data remains unavailable; it is never replaced with a guessed CPU,
640K RAM, a hardcoded shell or an inferred machine model. No subprocesses or
external network probes are used to collect information.

Networking uses a read-only local socket query to `ktcp`.

## Usage

```sh
elksftch
elksftch --mono
elksftch --plain
elksftch --compact --width 40
elksftch --host "Amstrad PPC 512"
elksftch --host "HP 200LX"
elksftch --logo wide --width 132
elksftch --help
```

Colour is automatic only on a terminal with a nonempty, non-`dumb` `TERM`;
`NO_COLOR` disables automatic colour. Use `--color always`, `--color never`
or `--mono` to override it. Redirected output has no automatic escape codes.
`--plain` selects information-only, uncoloured output. Text is sanitized to
printable ASCII, clipped to the selected width and kept out of the final
column to avoid terminal wrapping. No cursor positioning is required.

The 26-column compact logo fits beside the fields on an 80-column display.
Narrow screens use a stacked layout. The original 100-column banner is
available with `--logo wide` on terminals wider than 100 columns; it falls back
to the small layout when it does not fit.

The default width comes from the terminal window size, with an 80-column
fallback and `--width` override.

## Networking

With an active stack and an assigned address:

```text
Network: connected (configured)
IPv4: 10.0.2.15
Netmask: 255.255.255.0
Gateway: 10.0.2.2
```

With the stack stopped: `Network: disconnected`. A running stack with no IPv4
address shows `disconnected (no IPv4)`. Errors remain `unavailable` rather than
being mistaken for a disconnected network.

“Connected (configured)” means a live ELKS stack has an IPv4 address; it does
not certify Ethernet carrier, a SLIP peer, or Internet access. No ping, DNS
lookup or outgoing network connection is performed.

Older stacks, including the HP 200LX release 4.0 stack, can supply the IPv4
address through the socket API but lack the newer mask/gateway query. The reply timeout then adds up to
about two seconds. Saved `/etc/net.cfg` values are not used as live status.

See [network implementation and validation](docs/NETWORKING.md).

![ELKSfetch with networking configured in QEMU](docs/screenshots/elks-network.png)

*Development build in QEMU with NE2000 networking and a static guest address.
The version 0.1 capture appears below.*

## Screenshots

All captures below were taken on 2026-11-01. The HP images are ELXcap captures,
and the PPC images come from its actual text video memory and BIOS font.
The QEMU images are emulator framebuffer captures.
See [screenshot provenance](docs/screenshots/README.md) for build details and
original files. The latest HP capture appears at the top of this README, and
the networking development capture appears in the Networking section above.

### Amstrad PPC 512 — version 0.1

![ELKSfetch 0.1 running on the physical Amstrad PPC 512](docs/screenshots/ppc512-mono.png)

*The draft-release executable running on the PPC with PicoMEM, using `--mono`
and an explicitly configured model label. IPv4, netmask and gateway come from
the live ELKS network stack.*

![ELKSfetch 0.1 compact output on the physical Amstrad PPC 512](docs/screenshots/ppc512-compact.png)

*The same executable with `--plain --width 40`. Output occupies 40 columns of
the PPC's 80-column console; this does not switch its hardware video mode.*

These images reconstruct the PPC's logical text display from B8000h video
memory and its freshly captured BIOS font. The 200 scan lines are doubled to
640×400 for viewing. LCD tint/contrast and cursor/blink phase are not reproduced.
See the [PPC test report](docs/PPC512-20261101.md) and
[capture manifest](docs/PPC512-20261101.json) for evidence.

### HP 200LX DS — first hardware test

![First ELKSfetch hardware test on the HP 200LX DS](docs/screenshots/hp200lx-ds.png)

*The initial development build, before the network field was added. Captured
with ELXcap; original [640×200 BMP](docs/screenshots/hp200lx-ds.bmp) retained.*

### QEMU — version 0.1 with networking

![ELKSfetch version 0.1 with networking configured in QEMU](docs/screenshots/elks-v0.1.png)

*The exact version 0.1 executable attached to the draft release. Live network
configuration is shown with `--mono`.*

### QEMU — colour

![ELKSfetch running on the ELKS console in QEMU](docs/screenshots/elks-colour.png)

*Initial development build on the ELKS 0.9.2 console, 80 columns. QEMU is
configured as a 486 PC; the program is compiled for the 8086 instruction set.
The `Platform` line is
ELKS's build/platform string, not a detected CPU model.*

### QEMU — monochrome

![ELKSfetch monochrome console capture](docs/screenshots/elks-mono.png)

*Initial development build with `--mono`.*

### QEMU — compact

![ELKSfetch compact console capture](docs/screenshots/elks-compact.png)

*Initial development build with `--plain --width 40` on the same 80-column
QEMU console; this is not a photograph of a physical 40-column display.*

## Build with ELKS

With an already built ELKS checkout:

```sh
ELKSFETCH_ELKS=/path/to/elks sh toolchain/build-external.sh
```

The executable is `build/elksftch`; install it as `/bin/elksftch` on both FAT
and MINIX. Every installed filename fits FAT 8.3; no long-name creation or
filesystem-specific alias is required. All logos are embedded.
The build script uses the existing ELKS IA16 compiler,
libc and host tools, with no Python or checksum-tool prerequisite.

The included [integration patch](integration/elks-external.patch) follows
[ghaerr's external-repository guidance for ELKSWeb](https://github.com/ghaerr/elks/discussions/2185#discussioncomment-19053594):
`./buildext.sh elksfetch` clones/updates/builds the repository, and an
`ExtApplications` entry includes the binary in configured HD images. Default
floppy selections stay unchanged. The public repository can be cloned without
GitHub credentials; the integration patch remains an unsubmitted candidate.

See [building, remote compilation and image integration](docs/BUILDING.md)
and [validation evidence](docs/VALIDATION.md).

## Project layout

| Path | Purpose |
| --- | --- |
| `src/` | CLI, bounded formatting and ASCII layouts |
| `platform/` | ELKS system and network information collection |
| `toolchain/build-external.sh` | Minimal IA16 build for external application integration |
| `integration/` | Proposed upstream ELKS build/install patch |
| `artwork/` | Original wide wordmark and provenance |
| `tests/` | Host fixture checks and real ELKS/QEMU smoke test |
| `docs/screenshots/` | HP 200LX, PPC 512 and QEMU display captures |

The collector is separate from the renderer so a future DOSfetch can share
the presentation code while using DOS/BIOS interfaces. See the [roadmap](docs/ROADMAP.md).

## Credits and licence

Created by Alexander Rau (`l00nix`). Thanks to ghaerr and the ELKS contributors
for the operating system, toolchain, system interfaces and external application
build approach.

The presentation is inspired by [Neofetch](https://github.com/dylanaraps/neofetch).
No Neofetch source was copied. New source code is [MIT licensed](LICENSE).

See the [artwork notes](artwork/README.md) for the supplied wordmark's provenance.
