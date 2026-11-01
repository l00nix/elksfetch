# Roadmap

## First ELKS milestone

- [x] Native 8086 C executable, separate collector and renderer.
- [x] Small, wide and information-only layouts with monochrome support.
- [x] Native kernel identity, memory and uptime interfaces.
- [x] Minimal external build script and proposed ELKS image integration patch.
- [ ] Test on the Amstrad PPC 512 and HP 200LX; retain genuine captures.
- [ ] Set the first release version after hardware acceptance.
- [ ] Confirm artwork attribution and choose which screenshots to publish.
- [ ] Make the repository public when the owner requests it.
- [ ] Propose upstream integration after public clone access is available.

## DOSfetch sibling

Keep the CLI and renderer reusable. A future `platform/dos.c` would collect
DOS version, conventional memory, command interpreter and drive information.
DOS-family identification, BIOS reporting, EMS/XMS and video-adapter detection
need separate, explicit tests. DOS colours must work without assuming ANSI.SYS.
No DOS executable or DOS hardware support is claimed by this milestone.

## Later ELKS possibilities

Optional filesystem/network fields, additional logos and machine profiles.
Report CPU details only when they can be obtained reliably. `uname().machine`
is a kernel platform/build string, not a processor model probe. Avoid CPU-speed
benchmarks at startup and avoid modern Linux `/proc` or `/sys` assumptions.
