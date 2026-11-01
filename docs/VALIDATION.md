# Validation — 2026-11-01

This records the emulator/build milestone. A subsequent
[HP 200LX DS hardware smoke test](HP200LX-DS-20261101.md) passed on the same
date. This is still a development milestone, not a public release.

## Build

- Native target: 8086 instruction set, ELKS small memory model.
- Cross-compiler: `ia16-elf-gcc (GCC) 6.3.0`, on the existing Linux build host.
- Prepared ELKS build tree: `e1469b64c5809c44d5c822eaeebd0e97d5996289`.
- ELKS integration files: `7e0d8ea8ea87dbd4da768284887ad953be4bbaa0`.
- Application runtime source: initial commit `2918c06f092a924cbc6e5cd6a3ded1d9a2be1153`.
  Later documentation/test-helper changes do not change the runtime source.
- Compiler warnings: none.
- Executable: **9,712 bytes**, identified as a Linux-8086 `A_EXEC` executable.
- Reserved heap: 4096 bytes; reserved stack: 2048 bytes. File size is not the
  complete runtime memory requirement.
- SHA-256: `0c2de601af88122e218b52734a744788ca9d1c35e015709b615dc25ac080db7a`.

## External integration

The candidate patch passed `git apply --check` against the recorded ELKS
integration baseline. A separate harness used its actual `buildext.sh`,
`env.sh`, `ExtApplications` and `Make.install`, with links to the prepared
ELKS compiler/libc/header tree.

The harness tested the new clone, build, update, rebuild and selected install
paths. Git's URL rewrite mapped the private repository URL to a transferred
Git bundle containing the committed source, so no GitHub credentials were
needed on the build host. The URL in the candidate patch remains the eventual
repository URL. Two builds produced identical bytes. The installer selected
`:elksfetch`, placed the executable in `target/bin/elksfetch`, and its bytes
matched the standalone build. The unchanged upstream installer printed its
harmless missing-`sudo` chmod message because internal applications were not
part of this isolated install.

This verifies the selected ELKSfetch build/install path. It does not claim
that every external application, `buildext.sh all`, or a full fresh ELKS
kernel/image build was tested. The installed binary was then put into an
isolated copy of an existing FAT HD image for runtime checks.

## Runtime and screenshots

- QEMU PC, 486 CPU, TCG with `one-insn-per-tb=on`, 16 MiB guest memory.
- ELKS 0.9.2, kernel identifying commit `e1469b64`.
- Base FAT HD image SHA-256:
  `2302b8604974a0ace258f5a651cf04cad1b36c5b59fdf218f1024b254f18812d`.
- No network device, and no access to physical disks.
- Actual target binary executed through the guest shell.
- Kernel identity, memory fields, advancing uptime and terminal information
  checked; `meminfo -m` and `ps -u` output retained for comparison. Different
  processes consume different amounts of RAM, so free-memory totals are not
  expected to be identical between commands.
- Invalid width returns status 2. Redirected output contains no automatic
  ANSI escapes. Configured machine labels and compact output exercised.
- Short-name guest FAT copy exercised as `elkfetch`.
- The unchanged binary was executed in a guest chroot with no `/dev/kmem`
  or password database: memory/uptime were explicitly unavailable, the user
  fell back to `uid 0`, and the command exited successfully.

The three README images are QEMU framebuffer captures, converted losslessly
from PPM to PNG. They are not mockups or host-fixture renderings. They show
colour, monochrome and 40-column information-only output on the guest's
80-column console. The compact screenshot does not represent a physical
40-column display. The capture procedure and serial transcript accompany them.

## Host checks

Five test groups cover all 221 supported widths (20–240) with all three logo
choices, plain/colour behavior, long and control-character-containing values,
wide-logo fallback and invalid arguments. The fixture is explicitly labelled
and is never linked into the ELKS executable. Host compilation uses C89,
`-Wall -Wextra -Werror -pedantic`. Shell syntax checks pass.

The workflow draft is in `tests/github-host-checks.yml`. GitHub Actions is
not enabled because the upload credential lacks workflow scope. The checks
above were run locally; no successful GitHub Actions run is claimed.

## Remaining acceptance

The initial HP 200LX DS hardware run is documented separately. PPC 512,
true 8088 timing/performance, serial terminal varieties, longer-running
hardware acceptance, and other ELKS configurations remain unverified. This
QEMU run on a 486 does not establish execution on a physical 8086/8088.
DOSfetch is not implemented. The integration patch has not been submitted
upstream, and the repository remains private.
