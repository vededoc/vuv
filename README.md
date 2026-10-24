# vuv

![CI](https://github.com/vededoc/vuv/actions/workflows/ci.yml/badge.svg)

A modern C++20 wrapper around [libuv](https://libuv.org/).

`vuv` exposes libuv's event loop, handles, and requests through a small,
header-friendly C++ API while keeping the underlying `uv_*` types accessible
when you need them.

## Features

- RAII-friendly wrappers for common libuv primitives: `Loop`, `UvTcp`,
  `UvUdp`, `UvTimer`, `UvCheck`, `UvPrepare`, `UvPoll`, `UvSignal`,
  `UvPipe`, `UvTty`, `UvStream`, `UvAsync`, `Idle`, `Immediate`
- DNS helpers: `DNSResolver`, `UvGetAddrInfo`
- Process spawning via `UvSpawn`
- Utility helpers: `UvBufStore`, `HandleSender` / `HandleReceiver`
- CMake package config (`find_package(vuv)`) and install rules

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16+
- libuv development headers and library

On Debian/Ubuntu:

```sh
sudo apt-get install -y libuv1-dev cmake g++
```

## Build

```sh
cmake -S . -B build -DVUV_BUILD_TESTS=ON
cmake --build build
```

## Run tests

```sh
ctest --test-dir build --output-on-failure
```

## Install

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

## Use from another project

After installation:

```cmake
find_package(vuv 0.9 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE vuv::vuv)
```

```cpp
#include <vuv/vuv.h>

int main() {
    vuv::Loop::Open();
    // ... set up handles ...
    vuv::Loop::Run();
    vuv::Loop::Close();
    return 0;
}
```

## Packaging

CPack is configured. To produce distributable archives:

```sh
cd build
cpack
```

This generates `vuv-0.9.0-<system>.tar.gz` and `.zip` archives.

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md).
