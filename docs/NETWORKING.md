# Networking — 2026-11-01

ELKSfetch reports the state of the running ELKS IPv4 stack. It does not test
Internet reachability or physical carrier. A configured but unplugged Ethernet
adapter, or a SLIP link without a responding peer, can still appear configured.

## Collection

- `socket(AF_INET, SOCK_STREAM, 0)` returns `ENETDOWN` when `ktcp` is absent:
  `Network: disconnected`.
- Bind a temporary socket to the wildcard address and an ephemeral port.
  ELKS returns the stack's live local IPv4 through `getsockname()`, including
  on the HP 200LX release 4.0 sources. This is the older-stack fallback.
- Connect to the special local `0.0.0.0:2` netconf endpoint using ELKS libc's
  two-second `in_connect()` timeout. `ktcp` intercepts this connection locally;
  it is not sent to the network.
- Send only the two-byte read request `{NS_GET_CONFIG=6, 0}`. Receive the
  26-byte configuration reply, accepting partial reads. The address, mask and
  gateway occupy its first twelve bytes in network byte order. A reply deadline
  of approximately two seconds prevents waiting indefinitely for this optional
  command on older stacks; the live socket address remains available on timeout.
- Close the temporary socket. No network configuration is changed. No saved
  configuration file is treated as evidence that networking is running.

`connected (configured)` requires a nonzero live IPv4 address. A zero address
produces `disconnected (no IPv4)`. A zero gateway is shown as `none`. Resource
or bind errors produce `unavailable`; a successful bind with no usable status
produces `active (details unavailable)`. Unknown data is never manufactured.
The older protocol cannot supply netmask/gateway, so those rows are omitted.

Interface names and MAC addresses are intentionally omitted: some `ktcp`
versions return the default Ethernet device name even when using SLIP.
ELKS does not provide a reliable carrier flag through this protocol.

The timeout applies to connection/reply waiting. ELKS's synchronous socket
bind/write/read internals still depend on a responsive `ktcp`; these kernel
calls cannot all be bounded by a userspace timeout on the tested kernel.

Protocol references inspected: ELKS `elkscmd/ktcp/netconf.h`, `netconf.c`,
`tcpdev.c`, `elks/net/ipv4/af_inet.c`, and `libc/net/in_connect.c`, plus the
HP release 4.0 sources. The collector is independently written MIT code.

## Validation

Native build: existing remote Linux host, IA16 GCC 6.3.0, prepared ELKS tree
`e1469b64c5809c44d5c822eaeebd0e97d5996289`, using the same
`toolchain/build-external.sh` called by the external-build integration.
No compiler warnings. Executable: **10,944 bytes**.

- SHA-256: `cf328f5bd86754ff74e851a22df9d996409c7f31d358b6ac49baf201928fe879`
- MD5 for hardware transfer verification: `30a83c25d1a7f21ae2f4f1f2141add4f`
- Host collector tests: 13 scenarios including partial reply, EOF, timeout,
  missing newer protocol, resource/bind/connect errors, zero address/gateway,
  and a total reply deadline even when data arrives in small fragments.
- Renderer/CLI tests: six groups, all supported widths/logos and the maximum
  16 rows (model, XMS, all network fields and TTY included).
- Actual ELKS binary in QEMU: offline initially, live `10.0.2.15`, mask
  `255.255.255.0`, gateway `10.0.2.2` after starting `ktcp`, offline again after
  stopping it. Existing CLI, memory, uptime, redirected output, short FAT name
  and missing-device fallback checks also passed.
- Actual HP 200LX DS via BH00K9NP, 9600 8N1: uploaded as
  `/root/fetch01/netfetch`, verified MD5, and ran successfully under
  `hp200lx-4.0-cf`. Output showed `Network: disconnected`, consistent with no
  running `ktcp` in `ps`. No hardware networking was started or reconfigured.
- HP display captured with ELXcap 0.1 to `hpnet.bmp`; downloaded BMP MD5
  `450f338977eceab23a590bc17aca532a` matched the device. PNG is a lossless
  conversion. The HP's system `/bin/elksfetch` was not changed.

The older-stack active-network fallback is covered by host fixtures and source
inspection; it has not yet been exercised with an active link on the HP.
PPC hardware testing remains pending. Screenshot provenance and the QEMU
transcript are in [screenshots](screenshots/README.md).
