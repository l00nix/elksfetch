# Roadmap

## First ELKS milestone

- [x] Native 8086 C executable, separate collector and renderer.
- [x] Small, wide and information-only layouts with monochrome support.
- [x] Native kernel identity, memory and uptime interfaces.
- [x] Live network state, IPv4 and supported netmask/gateway queries.
- [x] Minimal external build script and proposed ELKS image integration patch.
- [x] HP 200LX DS smoke test and ELXcap screenshot retained (2026-11-01).
- [ ] Test on the Amstrad PPC 512 and broaden HP 200LX acceptance.
- [x] Prepare version 0.1 as a private draft release after initial HP acceptance.
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

Optional filesystem fields, additional logos and machine profiles.
Report CPU details only when they can be obtained reliably. `uname().machine`
is a kernel platform/build string, not a processor model probe. Avoid CPU-speed
benchmarks at startup and avoid modern Linux `/proc` or `/sys` assumptions.
