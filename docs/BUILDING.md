# Building and installing

## Prepared ELKS tree

Use a built [ELKS checkout](https://github.com/ghaerr/elks), including its IA16
compiler, libc and host tools. The normal ELKSfetch build requires only a POSIX
shell and that toolchain. Python, a native host compiler and QEMU are optional
verification tools; they are not runtime-build dependencies.

```sh
ELKSFETCH_ELKS=/absolute/path/to/elks sh toolchain/build-external.sh
```

`TOPDIR` is also accepted, matching ELKS's external-application environment.
The script uses `-march=i8086`, the small memory model, ELKS libc, 4096 bytes of
heap and 2048 bytes of stack. It rebuilds all four translation units, writes
`build/elksfetch` and `build/elksfetch.map`, and removes an old executable
before compiling so a failed compile cannot leave a stale install candidate.

Install `build/elksfetch` as `/bin/elksfetch` using the ELKS image builder or
host-side mtools (which creates its FAT long filename). When copying within
an ELKS FAT filesystem, use the eight-character name `/bin/elkfetch` and run
`elkfetch`: ELKS FAT file creation does not create long filenames. MINIX
filesystems can use the full `elksfetch` name. It has no required
runtime assets. The wide and small ASCII logos are embedded. A normal user can
run it; fields requiring `/dev/kmem` become explicitly unavailable if access
is denied. Do not make it setuid just to obtain those fields.

## Remote compilation

On a Linux or macOS build host with a prepared ELKS tree, clone this repository
into `extapps/elksfetch`, then invoke the same shell builder. For example:

```sh
ssh buildbox 'cd /work/elks/extapps/elksfetch && git pull --ff-only && TOPDIR=/work/elks sh toolchain/build-external.sh'
scp buildbox:/work/elks/extapps/elksfetch/build/elksfetch ./build/elksfetch
```

Create local `build/` first. Substitute your SSH host and checkout path.
The repository is public and can be cloned over HTTPS without GitHub
credentials. A source archive or Git bundle can also be transferred with SSH.

## ELKS external application and image inclusion

This follows the external build approach recommended by
[ghaerr for ELKSWeb](https://github.com/ghaerr/elks/discussions/2185#discussioncomment-19053594).
The candidate patch is in `integration/elks-external.patch` and was generated
against ELKS commit `7e0d8ea8ea87dbd4da768284887ad953be4bbaa0`.
It adds:

- `elksfetch()` and the `make_all()` entry in `buildext.sh`;
- clone/update of `extapps/elksfetch`;
- `elksfetch/build/elksfetch :elksfetch` in `elkscmd/ExtApplications`.

In a disposable/prepared ELKS checkout:

```sh
git apply --check /path/to/elksfetch/integration/elks-external.patch
git apply /path/to/elksfetch/integration/elks-external.patch
./buildext.sh elksfetch
```

The candidate deliberately has no floppy-size tags, matching ELKSWeb's
hard-disk-only default inclusion. ELKS's HD installation selects all external
application entries; an unbuilt external application is skipped by upstream's
installer. Build ELKSfetch first, then use the usual configured ELKS HD image
recipe, for example `make -C image hd32mbr-fat` in the sourced ELKS environment.
For a custom selection, `:elksfetch` is the application tag; fixed floppy
profiles override generic tags in the current upstream installer, so adding
it to a floppy requires an explicit profile/table change and a size check.

The patch is an integration candidate, not an upstream submission. Public
ELKS builds can now clone this repository without credentials. Applying or
submitting the integration patch remains a separate step.

## Verification

```sh
python3 tests/test_render.py
python3 tests/test_network.py
python3 tests/qemu_smoke.py --image /path/to/hd32-fat.img \
  --binary build/elksfetch --out test-output/qemu
```

The first command uses a labelled host fixture and tests all supported widths,
logos, control-character sanitization and invalid arguments. It does not run
ELKS. The second checks the network collector with host fixtures. The third
runs the target binary in QEMU and requires `qemu-system-i386`
and mtools. Supply an unpartitioned bootable FAT ELKS HD image with serial
getty support. The script copies the image into a temporary directory, installs
the binary only into that copy, retains serial logs and QEMU PPM screenshots,
and closes the emulator. It does not touch a physical disk or the input image.

The proposed host-check workflow is saved as `tests/github-host-checks.yml`.
It is not enabled: the GitHub credential used for this initial push does not
have workflow scope. The same checks have passed locally. An owner can later
move the draft to `.github/workflows/check.yml` using a credential with that
permission. Target-build/runtime evidence is recorded in `docs/VALIDATION.md`.

For the optional missing-device test, build a small test-only launcher using
the same prepared IA16 environment:

```sh
# With TOPDIR and the ELKS toolchain PATH already set:
ia16-elf-gcc -march=i8086 -mcmodel=small -melks-libc -Os -Wall -Wextra \
  -maout-stack=2048 -maout-heap=2048 tests/without_kmem.c -o build/fetchroot
python3 tests/qemu_smoke.py --image /path/to/hd32-fat.img \
  --binary build/elksfetch --chroot-helper build/fetchroot --out test-output/qemu
```

The launcher changes only its own process root to a temporary guest directory
without `/dev` or `/etc`, then executes the unchanged ELKSfetch binary. This
tests unavailable memory/uptime and UID fallback without altering kernel
devices. It is not installed with the application.

Network tests cover collector errors, partial replies, timeout and older-stack
fallback. The QEMU smoke test also starts/stops networking on a virtual NE2000
and checks live IPv4, netmask and gateway values. It uses a temporary image
and does not configure physical hardware or forward host ports.
