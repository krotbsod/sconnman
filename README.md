# Sconnman — Minimalistic Network Manager for Linux

**Sconnman** (Small Connection Manager) is a lightweight network manager with minimal dependencies, written in C++17. The project is in a **rudimentary (pre‑alpha)** state: only basic wrappers over Netlink for interface, address, and route management are implemented. The long‑term goal is to support a replaceable API core (Netlink, sysfs, ioctl, etc.), but currently only Netlink is used.

## Project Goals

- Full control over the network without tying to `glibc` (works with `uclibc`, `musl`).
- Minimum external dependencies – everything is built on direct Netlink calls (`socket`, `struct nlmsghdr`).
- Easy replacement of the transport layer: adding `rtnetlink` or `sysfs` emulation later.
- Avoid NetworkManager’s pitfalls (complexity, heavy dependencies).
- DHCP support via external `udhcpc` (BusyBox) – no built‑in client.

> **Current state**: only Netlink wrappers work – interface listing, address/route management. Connection logic (WiFi/Ethernet) is mostly conceptual.

## Features

- **C++17**
- **Linux only** – depends on `<linux/netlink.h>` and `AF_NETLINK`.
- **No glibc dependency** – tested with an `uclibc`‑based toolchain.
- **DHCP** – delegated to an external `udhcpc` process (not embedded).
- **Replaceable API core** – abstractions (`netlink.hpp`, `netctl.hpp`) designed to allow other backends in the future.

## Project State (Pre‑alpha)

| Component               | Implementation                                               |
|-------------------------|--------------------------------------------------------------|
| `NetlinkCtx`            | ✅ basic Netlink message send/receive                        |
| `InterfaceManager`      | ✅ list interfaces, link‑state events                        |
| `AddressManager`        | ✅ add/remove/list IP addresses on interfaces                |
| `RouteManager`          | ✅ IPv4/IPv6 route management                                |
| `Connection` (abstract) | ⚠️ interface defined, concrete implementations (`WiredConnection`) are rudimentary |
| DHCP client (`udhcpc`)  | ⚠️ invoked via `command_utils`, no full integration yet     |
| Example (`example/`)    | ✅ demonstrates basic Netlink operations (list interfaces, add address) |

> **Important**: API is unstable and may change drastically. Do not use in production.

## Building

### Dependencies

- CMake ≥ 3.16
- C++17 compiler (GCC 7+, Clang 5+)
- Linux kernel headers (`linux/netlink.h`, `linux/rtnetlink.h`)
- (optional) `udhcpc` – for testing DHCP

The project does **not** require `libnl`, `dbus`, `glib`, or other heavy libraries.

### Build the project

```bash
git clone https://github.com/krotbsod/sconnman.git
cd sconnman
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
