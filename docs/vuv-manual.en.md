# vuv Manual (English)

Version 0.9.0 — a modern C++20 wrapper around [libuv](https://libuv.org/).

`vuv` exposes libuv's event loop, handles, and requests through a small,
RAII-friendly C++ API while keeping the underlying `uv_*` types accessible
when you need them.

Umbrella header:

```cpp
#include <vuv/vuv.h>
```

(In-tree builds use `#include "vuv.h"`. After `cmake --install`,
use `<vuv/vuv.h>`.)

## Contents

1. Features
2. Requirements
3. Build
4. Run tests
5. Install (including this manual)
6. Use from another project
7. Quick start
8. Core concepts
9. API reference
10. Logging and utilities
11. Threading
12. Examples
13. Packaging
14. License and contributing

## 1. Features

- Event loop: `Loop` (per-thread singleton)
- Stream handles: `UvStream` (base), `UvTcp`, `UvPipe`, `UvTty`
- Datagram handle: `UvUdp`
- Polling/idle/prepare/check: `UvPoll`, `UvIdle`, `UvPrepare`, `UvCheck`
- Timers and deferred work: `UvTimer`, `Immediate`
- Cross-thread wakeup: `UvAsync`
- Signals: `UvSignal`
- Processes: `UvSpawn`
- DNS: `DNSResolver`, `UvGetAddrInfo`
- Utilities: `UvBufStore`, `HandleSender` / `HandleReceiver`
- CMake package config (`find_package(vuv)`) and install rules

## 2. Requirements

- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16+
- libuv development headers and library

On Debian/Ubuntu:

```sh
sudo apt-get install -y libuv1-dev cmake g++
```

## 3. Build

```sh
cmake -S . -B build -DVUV_BUILD_TESTS=ON
cmake --build build
```

Options (both default to `OFF`):

| Option | Effect |
| --- | --- |
| `VUV_BUILD_TESTS` | Builds `tests/` and enables `ctest` |
| `VUV_BUILD_EXAMPLES` | Builds `examples/vuv_tcp_echo_timer` |

Memory-check build:

```sh
cmake -S . -B build -DMEM_CHECK=ON
cmake --build build
```

This adds `-fsanitize=leak,address -lasan`.

## 4. Run tests

```sh
ctest --test-dir build --output-on-failure
```

The test suite uses GoogleTest (fetched automatically when
`VUV_BUILD_TESTS=ON`) and covers loop lifecycle, timers, async, idle,
check/prepare, poll, signals, DNS, UDP, TCP, buffers, and process spawn.

## 5. Install (including this manual)

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

Installed layout (with default `GNUInstallDirs` paths):

| Artifact | Location |
| --- | --- |
| Library | `<prefix>/lib/libvuv.a` |
| Headers | `<prefix>/include/vuv/*.h` |
| CMake config | `<prefix>/lib/cmake/vuv/vuvConfig*.cmake`, `vuvTargets.cmake` |
| This manual (Markdown) | `<prefix>/share/doc/vuv/vuv-manual.en.md` |
| This manual (man page) | `<prefix>/share/man/man7/vuv.7` |

After installation, read the manual with:

```sh
man 7 vuv
# or
less /usr/local/share/doc/vuv/vuv-manual.en.md
```

To install to a staging directory instead of the live system:

```sh
DESTDIR=/tmp/vuv-stage cmake --install build
```

The manual files are part of the installed file set, so they are also
included in the `cpack` archives (see section 13).

Custom install directories remain supported, e.g.:

```sh
cmake -S . -B build \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCMAKE_INSTALL_DOCDIR=share/doc/vuv \
  -DCMAKE_INSTALL_MANDIR=share/man
```

## 6. Use from another project

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

`vuv::vuv` already links `uv`, `pthread`, and `dl` publicly, so downstream
targets do not need to link them explicitly.

## 7. Quick start

TCP client/server with a periodic timer (see
`examples/tcp_echo_timer.cpp`):

```cpp
#include "vuv.h"
using namespace vuv;

int main() {
    Loop::Open(nullptr);  // owned loop

    UvTcp server, client;
    server.init();
    client.init();
    server.bindAndListen(0, "127.0.0.1");

    UvTimer timer;
    timer.init();
    timer.setOn([&] { /* ... */ });
    timer.start(200, 200);  // first fire in 200 ms, then every 200 ms

    // ... setOnListen / accept / connect / readStart / write ...
    Loop::Run();    // UV_RUN_DEFAULT until no active handles remain
    Loop::Close();  // frees the owned loop
    return 0;
}
```

Build and run the bundled example:

```sh
cmake -S . -B build -DVUV_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/vuv_tcp_echo_timer
```

## 8. Core concepts

### Event loop ownership

`vuv::Loop` is a per-thread singleton. Typical lifecycle:

```
Loop::Open() -> init/start/connect/... -> Loop::Run([mode]) -> stop()/close() -> Loop::Close()
```

- `Loop::Open(uv_loop_t *loop = nullptr)`: no loop must exist on this
  thread yet. With `nullptr` it allocates and owns a new `uv_loop_t`;
  with a non-null pointer it borrows an external loop.
- `Loop::Run(mode = UV_RUN_DEFAULT)`: runs `uv_run()` on the thread-local
  loop. `UV_RUN_ONCE` / `UV_RUN_NOWAIT` are useful for tests.
- `Loop::Close()`: closes the loop. Owned loops are retried with
  `uv_run(UV_RUN_NOWAIT)` on `UV_EBUSY` and freed on success.
- `Loop::GetUvLoop()`, `Loop::Get()`, `Loop::tick()` (millisecond
  `uv_now()`), `Loop::getEventHandles()` (handle census via `uv_walk`),
  and `Loop::CheckLoop()` (asserts handle/loop affinity) are helpers for
  diagnostics and internal checks.

### Handle lifecycle

Most handles follow `init()` -> `start()`/`connect()`/`bind()` ->
`stop()` -> `close()`. Destructors of `UvUdp`, `UvTimer`, `UvIdle`,
`UvCheck`, `UvPrepare`, `UvPoll`, `UvAsync`, and `UvSignal` close
automatically if still open; `UvStream`-derived handles expose `close()`
explicitly. Closing is backed by `uv_close()` and frees the native handle
asynchronously. Do not reuse a handle after `close()` without calling
`init()` again, and always stop/close handles before `Loop::Close()` so
the loop is not busy.

### Callbacks

Callbacks are `std::function` setters such as `setOn()`,
`setOnReadLis()`, `setOnCnnLis()`, `setOnEvent()`, and `setOnExit()`.
Some callbacks fire on the loop thread only; `UvAsync::send()` is the
exception and is safe to call from any thread.

### Raw libuv access

When the wrapper is not enough, each handle exposes its native type
(`getRawHandle()`, `getHandle()` depending on the class) plus the
`GET_RAWH` / `ASSERT_RAW_UVHANDLE` / `GET_UVHANDLE_OWNER` helpers in
`uvcppdef.h`. This keeps escape hatches explicit and local.

## 9. API reference

Signatures below are abbreviated; see the named headers for exact types.

### Loop — `src/Loop.h`

Per-thread singleton event loop.

- `static void Open(uv_loop_t *loop = nullptr)`
- `static void Close()`
- `static Loop *Get()`
- `static int Run(uv_run_mode mode = UV_RUN_DEFAULT)`
- `static uv_loop_t *GetUvLoop()`
- `static bool IsRunning()`
- `uv_loop_t *getLoop()`
- `int64_t tick()` — `uv_now()` in milliseconds
- `std::unordered_map<std::string,int> getEventHandles()`
- `static void CheckLoop(uv_loop_t *loop)`

All handles implicitly use `Loop::GetUvLoop()` during `init()`.

### UvStream — `src/UvStream.h` (base of TCP/Pipe/TTY)

Stream-oriented base with read/write/listen/accept.

- Types: `ReadCb = fn(const char*, size_t)`, `CnnCb = fn(int)`,
  `ListenCb = fn(UvStream&)`
- `setOnCnnLis()`, `setOnReadLis()`, `setOnWriteLis()`, `setOnListen()`
- `readStart()`, `readStop()`, `listen(backlog = 128)`,
  `accept(UvStream*)`
- `write(const char*, size_t)`, `write(string)`,
  `write2(UvStream* send_handle)` (fd passing, sends `"PING"`)
- `isReadable()`, `isWritable()`, `getWriteQueCnt()`,
  `getSendQueCnt()`, `close()`, `getRawHandle()` (`uv_stream_t*`)

Read errors/EOF close the native handle and report `CnnCb(-1)`.

### UvTcp — `src/UvTcp.h`

TCP handle.

- `init()`
- `connect(string, int)`, `connect(sockaddr*)` — auto-`readStart()`
  on success, always fires the connection callback
- `bind(sockaddr*, flags)`, `bind(port, addr)`
- `bindAndListen(port, ip, backlog = 128)`
- `bindAndListenReusePort(...)` — `SO_REUSEADDR`/`SO_REUSEPORT`
- `noDelay()`, `keepAlive()`, `simultaneousAccepts()`
- `getSockName()`, `getPeerName()`, `getPeerIpPort(string*, int*)`

```cpp
vuv::UvTcp c;
c.init();
c.setOnCnnLis([](int status) { /* 0 = connected */ });
c.connect("127.0.0.1", 8080);
```

### UvUdp — `src/UvUdp.h`

UDP handle (standalone; not an `UvStream`).

- `init()`, `initEx(flags)`, `close()`
- `bind(sockaddr*, flags)`, `bind(ip, port, flags)` (v4/v6 auto-detect)
- `send(uint8_t*, len, addr)`, `send(string, addr)`
- `recvStart()`, `recvStop()`
- `setOnRecvLis(RecvLis(buf, len, addr, flags))`,
  `setOnSendLis(SendLis(int))`
- `getSockName()`, `setMemberShip()`, `setMulticastLoop/Ttl/Interface()`,
  `setBroadcast()`, `setTtl()`, `getRawHandle()` (`uv_udp_t*`)

```cpp
vuv::UvUdp u;
u.init();
u.bind("0.0.0.0", 9000, 0);
u.setOnRecvLis([](auto b, auto n, auto a, auto f) { /* ... */ });
u.recvStart();
```

### UvPipe — `src/UvPipe.h`

Unix-domain / named-pipe handle.

- `init(ipc)`, `open(uv_file)`, `bind(name)`, `connect(name)`
- `getSockName()`, `getPeerName()`, `pendingInstances(count)`,
  `pendingCount()`, `pendingType()`

```cpp
vuv::UvPipe p;
p.init(0);
p.setOnCnnLis([](int status) { /* ... */ });
p.connect("/tmp/app.sock");
```

### UvTty — `src/UvTty.h`

Terminal handle.

- `UvTty(fd = 0, readable = 1)`, `init()`
- `setMode(uv_tty_mode_t)`, `resetMode()`, `getWinSize(w, h*)`

```cpp
vuv::UvTty t(1, 0);
t.init();
t.write("out\n");
```

### UvTimer — `src/UvTimer.h`

One-shot/repeating timer.

- `init()`, `setOn(void())`, `start(timeout, interval)`,
  `stop()`, `again()`, `setRepeat(uint64_t)` (0 also stops),
  `getRepeat()`, `close()`, `getRawHandle()`

```cpp
vuv::UvTimer t;
t.init();
t.setOn([] { /* fired */ });
t.start(100, 1000);
```

### UvIdle — `src/UvIdle.h`

Runs on every loop iteration.

- `init()`, `setOn(void())`, `start()` (idempotent), `stop()`,
  `deinit()` (note: named `deinit`, not `close()`), `getHandle()`

### Immediate — `src/Immediate.h`

`setImmediate` queue layered on `UvIdle`.

- `uint32_t setImmediate(ImmdCb)` — returns an odd handle id
- `clearImmediate(uint32_t)` — neuters a pending callback
- `clearAll()`, `stop()` — call `stop()` before destruction
- Re-entrant `setImmediate()` calls run on the next iteration.

```cpp
vuv::Immediate im;
auto h = im.setImmediate([] { /* next idle */ });
im.clearImmediate(h);
im.stop();
```

### UvCheck — `src/UvCheck.h`

Runs after each I/O poll.

- `init()`, `setOn(void())`, `start()`, `stop()`, `close()`

### UvPrepare — `src/UvPrepare.h`

Runs before each I/O poll.

- `init()`, `start(void())` (callback passed directly, no `setOn`),
  `stop()`, `close()`, `getRawHandle()`

### UvPoll — `src/UvPoll.h`

File-descriptor / socket readiness.

- `init(fd)`, `initSocket(fd)`
- `setOnEvent(fn(int events))` — only `events` is forwarded
  (`UV_READABLE` / `UV_WRITABLE` / `UV_DISCONNECT`)
- `start(events)`, `stop()`, `close()`, `getRawHandle()`

```cpp
vuv::UvPoll p;
p.init(fd);
p.setOnEvent([](int ev) { if (ev & UV_READABLE) { /* ... */ } });
p.start(UV_READABLE);
```

### UvAsync — `src/UvAsync.h`

Thread-safe loop wakeup. Coalesced: N `send()` calls produce >= 1
callback invocation.

- `init()`, `setOn(void())`, `send()` (callable from any thread),
  `close()`, `getRawHandle()`

### UvSignal — `src/UvSignal.h`

POSIX signal handling.

- `init()`, `start(signum)` (persistent),
  `startOneShot(signum)`, `setOn(fn(int signum))`,
  `stop()`, `close()`

```cpp
vuv::UvSignal s;
s.init();
s.setOn([](int sig) { /* ... */ });
s.start(SIGINT);
```

### UvSpawn — `src/UvSpawn.h`

Child-process spawning with piped stdout/stderr capture.

- `run(vector<string> args)`, `processKill(signum)`, `close()`
- `setOnConsoleOut(stdoutCb, stderrCb)` — each
  `fn(const char *buf, size_t size)`
- `setOnExit(fn(int exitCode))`

```cpp
vuv::UvSpawn sp;
sp.setOnConsoleOut([](auto b, auto n) { /* stdout */ },
                   [](auto b, auto n) { /* stderr */ });
sp.setOnExit([](int code) { /* ... */ });
sp.run({"/bin/echo", "hello"});
```

### DNSResolver — `src/DNSResolver.h`

Higher-level asynchronous resolver (not re-exported by `vuv.h`;
include `DNSResolver.h` directly).

- `query(node, QueryCb)` — `fn(int, unordered_set<string> v4,
  unordered_set<string> v6)`, returns request id
- `query(node, QueryNetCb)` — `fn(int, vector<sockaddr_storage> v4,
  vector<sockaddr_storage> v6)`, returns request id
- `abortReq(req_id)`, `close()`

### UvGetAddrInfo — `src/UvGetAddrInfo.h`

Thin `uv_getaddrinfo` wrapper (not re-exported by `vuv.h`; include
`UvGetAddrInfo.h` directly).

- `init()`
- `getAddrInfo(node, service, hints, Lis)` —
  `Lis = fn(int, struct addrinfo*)`
- `close()`

### UvBufStore — `src/UvBufStore.h`

Pooled read/write buffer allocator used internally by streams and UDP.

- `alloc(dsize) -> char*`, `recycle(char*)`,
  `setMaxBufSize(cap)` (default 10 MiB)

### HandleSender / HandleReceiver — `src/HandleSender.h`, `src/HandleReceiver.h`

File-descriptor passing over IPC pipes.

Sender (client):

- `connect(path)`, `close()`, `sendHandle(UvStream*)`

Receiver (server):

- `open(pipename, Lis)` — `Lis = fn(UvPipe*, uv_handle_type)`
- `close()` — also unlinks the socket path on Linux

## 10. Logging and utilities

- `vuv::UvStrErr(int err)` (`src/etc.h`) — `uv_strerror` wrapper without
  the static-string leak; uses a shared static buffer (not reentrant).
- Log levels: `VERBOSE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `SILENT`
  (`src/logdef.h`); macros `ule/ulw/uli/uld/ulv(FMT, ...)` emit
  timestamped `file:line` messages.
- `SetLogLevel(int)` / `UVC_SET_LOG_LEVEL(L)` control verbosity;
  compile-time `UVCLOG_LEVEL` defaults to `VERBOSE`.

## 11. Threading

- One `Loop` per thread (`thread_local`). Do not share handles across
  threads except through `UvAsync::send()`.
- `UvStrErr()` shares a static buffer and is not thread-safe.
- `Immediate` is loop-thread only; use `UvAsync` to wake the loop from
  worker threads and then schedule immediate work.

## 12. Examples

Beyond the TCP echo + timer example in section 7:

Timer:

```cpp
vuv::UvTimer t;
t.init();
t.setOn([] { /* ... */ });
t.start(100, 1000);
t.stop();
t.close();
```

Cross-thread wakeup:

```cpp
vuv::UvAsync a;
a.init();
a.setOn([] { /* runs on the loop thread */ });
// from any thread:
a.send();
```

UDP:

```cpp
vuv::UvUdp u;
u.init();
u.bind("0.0.0.0", 9000, 0);
u.setOnRecvLis([](auto buf, auto len, auto addr, auto flags) { /* ... */ });
u.recvStart();
```

Process spawn:

```cpp
vuv::UvSpawn sp;
sp.setOnConsoleOut([](auto b, auto n) {}, [](auto b, auto n) {});
sp.setOnExit([](int code) {});
sp.run({"/bin/echo", "hello"});
```

## 13. Packaging

CPack is configured. To produce distributable archives (which include
this manual via the install rules):

```sh
cd build
cpack
```

This generates `vuv-0.9.0-<system>.tar.gz` and `.zip` archives.

## 14. License and contributing

Distributed under the MIT License. See `LICENSE` for details.

Contributions are welcome. See `CONTRIBUTING.md`.
