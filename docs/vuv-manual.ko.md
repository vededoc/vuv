# vuv 매뉴얼 (한국어)

버전 0.9.0 — [libuv](https://libuv.org/)의 현대적인 C++20 래퍼입니다.

`vuv`는 libuv의 이벤트 루프, 핸들, 요청을 작고 RAII 친화적인 C++ API로
노출하면서, 필요할 때는 내부의 `uv_*` 타입에 직접 접근할 수 있게 합니다.

통합 헤더:

```cpp
#include <vuv/vuv.h>
```

(저장소 안에서 빌드할 때는 `#include "vuv.h"`를 사용합니다.
`cmake --install` 이후에는 `<vuv/vuv.h>`를 사용하세요.)

## 목차

1. 주요 기능
2. 요구 사항
3. 빌드
4. 테스트 실행
5. 설치 (이 매뉴얼 포함)
6. 다른 프로젝트에서 사용
7. 빠른 시작
8. 핵심 개념
9. API 레퍼런스
10. 로깅과 유틸리티
11. 스레딩
12. 예제
13. 패키징
14. 라이선스와 기여

## 1. 주요 기능

- 이벤트 루프: `Loop` (스레드별 싱글턴)
- 스트림 핸들: `UvStream` (기반), `UvTcp`, `UvPipe`, `UvTty`
- 데이터그램 핸들: `UvUdp`
- 폴링/아이들/프리페어/체크: `UvPoll`, `UvIdle`, `UvPrepare`, `UvCheck`
- 타이머와 지연 작업: `UvTimer`, `Immediate`
- 스레드 간 깨우기: `UvAsync`
- 시그널: `UvSignal`
- 프로세스: `UvSpawn`
- DNS: `DNSResolver`, `UvGetAddrInfo`
- 유틸리티: `UvBufStore`, `HandleSender` / `HandleReceiver`
- CMake 패키지 설정 (`find_package(vuv)`) 및 설치 규칙

## 2. 요구 사항

- C++20 호환 컴파일러 (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16+
- libuv 개발 헤더 및 라이브러리

Debian/Ubuntu:

```sh
sudo apt-get install -y libuv1-dev cmake g++
```

## 3. 빌드

```sh
cmake -S . -B build -DVUV_BUILD_TESTS=ON
cmake --build build
```

옵션 (둘 다 기본값은 `OFF`):

| 옵션 | 효과 |
| --- | --- |
| `VUV_BUILD_TESTS` | `tests/`를 빌드하고 `ctest`를 활성화 |
| `VUV_BUILD_EXAMPLES` | `examples/vuv_tcp_echo_timer`를 빌드 |

메모리 검사 빌드:

```sh
cmake -S . -B build -DMEM_CHECK=ON
cmake --build build
```

`-fsanitize=leak,address -lasan`이 추가됩니다.

## 4. 테스트 실행

```sh
ctest --test-dir build --output-on-failure
```

테스트 스위트는 GoogleTest를 사용하며 (`VUV_BUILD_TESTS=ON`일 때 자동
가져옴), 루프 생명주기, 타이머, async, idle, check/prepare, poll,
시그널, DNS, UDP, TCP, 버퍼, 프로세스 스폰을 다룹니다.

## 5. 설치 (이 매뉴얼 포함)

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

설치 결과물 (기본 `GNUInstallDirs` 경로 기준):

| 산출물 | 위치 |
| --- | --- |
| 라이브러리 | `<prefix>/lib/libvuv.a` |
| 헤더 | `<prefix>/include/vuv/*.h` |
| CMake 설정 | `<prefix>/lib/cmake/vuv/vuvConfig*.cmake`, `vuvTargets.cmake` |
| 이 매뉴얼 (Markdown, 영문) | `<prefix>/share/doc/vuv/vuv-manual.en.md` |
| 이 매뉴얼 (Markdown, 국문) | `<prefix>/share/doc/vuv/vuv-manual.ko.md` |
| man 페이지 | `<prefix>/share/man/man7/vuv.7` |

설치 후 매뉴얼 확인 방법:

```sh
man 7 vuv
# 또는
less /usr/local/share/doc/vuv/vuv-manual.ko.md
less /usr/local/share/doc/vuv/vuv-manual.en.md
```

실제 시스템 대신 스테이징 디렉터리에 설치하려면:

```sh
DESTDIR=/tmp/vuv-stage cmake --install build
```

매뉴얼 파일은 설치 파일 집합에 포함되므로 `cpack` 아카이브에도 함께
들어갑니다 (13절 참고).

사용자 지정 설치 디렉터리도 계속 지원됩니다. 예:

```sh
cmake -S . -B build \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCMAKE_INSTALL_DOCDIR=share/doc/vuv \
  -DCMAKE_INSTALL_MANDIR=share/man
```

## 6. 다른 프로젝트에서 사용

설치 후:

```cmake
find_package(vuv 0.9 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE vuv::vuv)
```

```cpp
#include <vuv/vuv.h>

int main() {
    vuv::Loop::Open();
    // ... 핸들 설정 ...
    vuv::Loop::Run();
    vuv::Loop::Close();
    return 0;
}
```

`vuv::vuv`가 `uv`, `pthread`, `dl`을 public으로 이미 링크하므로,
사용 측 타깃에서 따로 링크할 필요가 없습니다.

## 7. 빠른 시작

주기 타이머가 있는 TCP 클라이언트/서버 (전체 코드는
`examples/tcp_echo_timer.cpp` 참고):

```cpp
#include "vuv.h"
using namespace vuv;

int main() {
    Loop::Open(nullptr);  // 소유권을 가진 루프 생성

    UvTcp server, client;
    server.init();
    client.init();
    server.bindAndListen(0, "127.0.0.1");

    UvTimer timer;
    timer.init();
    timer.setOn([&] { /* ... */ });
    timer.start(200, 200);  // 200ms 후 첫 실행, 이후 200ms 간격 반복

    // ... setOnListen / accept / connect / readStart / write ...
    Loop::Run();    // 활성 핸들이 없을 때까지 UV_RUN_DEFAULT로 실행
    Loop::Close();  // 소유한 루프를 해제
    return 0;
}
```

동봉된 예제 빌드 및 실행:

```sh
cmake -S . -B build -DVUV_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/vuv_tcp_echo_timer
```

## 8. 핵심 개념

### 이벤트 루프 소유권

`vuv::Loop`는 스레드별 싱글턴입니다. 일반적인 생명주기:

```
Loop::Open() -> init/start/connect/... -> Loop::Run([mode]) -> stop()/close() -> Loop::Close()
```

- `Loop::Open(uv_loop_t *loop = nullptr)`: 같은 스레드에 루프가 없어야
  합니다. `nullptr`이면 새로 `uv_loop_t`를 할당해서 소유하고,
  null이 아니면 외부 루프를 빌려 씁니다.
- `Loop::Run(mode = UV_RUN_DEFAULT)`: 스레드-로컬 루프에 대해
  `uv_run()`을 실행합니다. 테스트에서는 `UV_RUN_ONCE` /
  `UV_RUN_NOWAIT`가 유용합니다.
- `Loop::Close()`: 루프를 닫습니다. 소유한 루프가 `UV_EBUSY`이면
  `uv_run(UV_RUN_NOWAIT)`으로 재시도한 뒤 성공 시 해제합니다.
- `Loop::GetUvLoop()`, `Loop::Get()`, `Loop::tick()` (밀리초 단위
  `uv_now()`), `Loop::getEventHandles()` (`uv_walk` 기반 핸들 현황),
  `Loop::CheckLoop()` (핸들/루프 소속 확인용 assert)는 진단 및 내부
  검사용 헬퍼입니다.

### 핸들 생명주기

대부분의 핸들은 `init()` -> `start()`/`connect()`/`bind()` ->
`stop()` -> `close()` 순서를 따릅니다. `UvUdp`, `UvTimer`, `UvIdle`,
`UvCheck`, `UvPrepare`, `UvPoll`, `UvAsync`, `UvSignal`의 소멸자는
열려 있으면 자동으로 닫고, `UvStream` 계열은 `close()`를 명시적으로
호출합니다. 닫기는 `uv_close()` 기반이며 네이티브 핸들을 비동기로
해제합니다. `close()` 후에는 `init()` 없이 재사용하지 마세요. 그리고
`Loop::Close()` 전에 핸들을 먼저 stop/close해서 루프가 busy 상태로
남지 않게 하세요.

### 콜백

콜백은 `setOn()`, `setOnReadLis()`, `setOnCnnLis()`,
`setOnEvent()`, `setOnExit()` 같은 `std::function` 세터로 등록합니다.
대부분의 콜백은 루프 스레드에서만 실행되며, 예외는 `UvAsync::send()`로
어떤 스레드에서든 호출할 수 있습니다.

### libuv 원시 핸들 접근

래퍼로 부족할 때는 각 핸들이 네이티브 타입을 노출합니다 (클래스에 따라
`getRawHandle()`, `getHandle()`). `uvcppdef.h`의 `GET_RAWH` /
`ASSERT_RAW_UVHANDLE` / `GET_UVHANDLE_OWNER` 헬퍼와 함께 사용하면
탈출 경로를 명시적이고 국소적으로 유지할 수 있습니다.

## 9. API 레퍼런스

아래 시그니처는 요약이며, 정확한 타입은 각 헤더를 참고하세요.

### Loop — `src/Loop.h`

스레드별 싱글턴 이벤트 루프.

- `static void Open(uv_loop_t *loop = nullptr)`
- `static void Close()`
- `static Loop *Get()`
- `static int Run(uv_run_mode mode = UV_RUN_DEFAULT)`
- `static uv_loop_t *GetUvLoop()`
- `static bool IsRunning()`
- `uv_loop_t *getLoop()`
- `int64_t tick()` — 밀리초 단위 `uv_now()`
- `std::unordered_map<std::string,int> getEventHandles()`
- `static void CheckLoop(uv_loop_t *loop)`

모든 핸들은 `init()` 시점에 암묵적으로 `Loop::GetUvLoop()`을 사용합니다.

### UvStream — `src/UvStream.h` (TCP/Pipe/TTY의 기반)

읽기/쓰기/listen/accept를 제공하는 스트림 기반 클래스.

- 타입: `ReadCb = fn(const char*, size_t)`, `CnnCb = fn(int)`,
  `ListenCb = fn(UvStream&)`
- `setOnCnnLis()`, `setOnReadLis()`, `setOnWriteLis()`, `setOnListen()`
- `readStart()`, `readStop()`, `listen(backlog = 128)`,
  `accept(UvStream*)`
- `write(const char*, size_t)`, `write(string)`,
  `write2(UvStream* send_handle)` (fd 전달, `"PING"` 전송)
- `isReadable()`, `isWritable()`, `getWriteQueCnt()`,
  `getSendQueCnt()`, `close()`, `getRawHandle()` (`uv_stream_t*`)

읽기 오류/EOF 시 네이티브 핸들을 닫고 `CnnCb(-1)`을 보고합니다.

### UvTcp — `src/UvTcp.h`

TCP 핸들.

- `init()`
- `connect(string, int)`, `connect(sockaddr*)` — 성공 시 자동으로
  `readStart()`하고, 항상 연결 콜백을 호출
- `bind(sockaddr*, flags)`, `bind(port, addr)`
- `bindAndListen(port, ip, backlog = 128)`
- `bindAndListenReusePort(...)` — `SO_REUSEADDR`/`SO_REUSEPORT`
- `noDelay()`, `keepAlive()`, `simultaneousAccepts()`
- `getSockName()`, `getPeerName()`, `getPeerIpPort(string*, int*)`

```cpp
vuv::UvTcp c;
c.init();
c.setOnCnnLis([](int status) { /* 0 = 연결됨 */ });
c.connect("127.0.0.1", 8080);
```

### UvUdp — `src/UvUdp.h`

UDP 핸들 (독립 클래스이며 `UvStream`이 아님).

- `init()`, `initEx(flags)`, `close()`
- `bind(sockaddr*, flags)`, `bind(ip, port, flags)` (v4/v6 자동 판별)
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

Unix-domain / 네임드 파이프 핸들.

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

터미널 핸들.

- `UvTty(fd = 0, readable = 1)`, `init()`
- `setMode(uv_tty_mode_t)`, `resetMode()`, `getWinSize(w, h*)`

```cpp
vuv::UvTty t(1, 0);
t.init();
t.write("out\n");
```

### UvTimer — `src/UvTimer.h`

단발성/반복 타이머.

- `init()`, `setOn(void())`, `start(timeout, interval)`,
  `stop()`, `again()`, `setRepeat(uint64_t)` (0이면 정지도 함께 수행),
  `getRepeat()`, `close()`, `getRawHandle()`

```cpp
vuv::UvTimer t;
t.init();
t.setOn([] { /* 실행됨 */ });
t.start(100, 1000);
```

### UvIdle — `src/UvIdle.h`

매 루프 반복마다 실행됩니다.

- `init()`, `setOn(void())`, `start()` (멱등), `stop()`,
  `deinit()` (참고: `close()`가 아니라 `deinit()`), `getHandle()`

### Immediate — `src/Immediate.h`

`UvIdle` 위에 구현된 `setImmediate` 큐.

- `uint32_t setImmediate(ImmdCb)` — 홀수 핸들 id 반환
- `clearImmediate(uint32_t)` — 대기 중인 콜백을 무력화
- `clearAll()`, `stop()` — 소멸 전에 `stop()` 호출 필요
- 재진입 `setImmediate()` 호출은 다음 반복에 실행됩니다.

```cpp
vuv::Immediate im;
auto h = im.setImmediate([] { /* 다음 idle에 실행 */ });
im.clearImmediate(h);
im.stop();
```

### UvCheck — `src/UvCheck.h`

매 I/O 폴 이후에 실행됩니다.

- `init()`, `setOn(void())`, `start()`, `stop()`, `close()`

### UvPrepare — `src/UvPrepare.h`

매 I/O 폴 이전에 실행됩니다.

- `init()`, `start(void())` (콜백을 직접 전달, `setOn` 없음),
  `stop()`, `close()`, `getRawHandle()`

### UvPoll — `src/UvPoll.h`

파일 디스크립터 / 소켓 준비 상태 감시.

- `init(fd)`, `initSocket(fd)`
- `setOnEvent(fn(int events))` — `events`만 전달됨
  (`UV_READABLE` / `UV_WRITABLE` / `UV_DISCONNECT`)
- `start(events)`, `stop()`, `close()`, `getRawHandle()`

```cpp
vuv::UvPoll p;
p.init(fd);
p.setOnEvent([](int ev) { if (ev & UV_READABLE) { /* ... */ } });
p.start(UV_READABLE);
```

### UvAsync — `src/UvAsync.h`

스레드 안전한 루프 깨우기. 합쳐짐(coalesced): `send()` N번 호출에
대해 콜백이 1번 이상 실행됩니다.

- `init()`, `setOn(void())`, `send()` (어떤 스레드에서든 호출 가능),
  `close()`, `getRawHandle()`

### UvSignal — `src/UvSignal.h`

POSIX 시그널 처리.

- `init()`, `start(signum)` (지속),
  `startOneShot(signum)`, `setOn(fn(int signum))`,
  `stop()`, `close()`

```cpp
vuv::UvSignal s;
s.init();
s.setOn([](int sig) { /* ... */ });
s.start(SIGINT);
```

### UvSpawn — `src/UvSpawn.h`

stdout/stderr 캡처가 가능한 자식 프로세스 실행.

- `run(vector<string> args)`, `processKill(signum)`, `close()`
- `setOnConsoleOut(stdoutCb, stderrCb)` — 각각
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

고수준 비동기 리졸버 (`vuv.h`에서 재노출하지 않으므로
`DNSResolver.h`를 직접 포함).

- `query(node, QueryCb)` — `fn(int, unordered_set<string> v4,
  unordered_set<string> v6)`, 요청 id 반환
- `query(node, QueryNetCb)` — `fn(int, vector<sockaddr_storage> v4,
  vector<sockaddr_storage> v6)`, 요청 id 반환
- `abortReq(req_id)`, `close()`

### UvGetAddrInfo — `src/UvGetAddrInfo.h`

`uv_getaddrinfo`의 얇은 래퍼 (`vuv.h`에서 재노출하지 않으므로
`UvGetAddrInfo.h`를 직접 포함).

- `init()`
- `getAddrInfo(node, service, hints, Lis)` —
  `Lis = fn(int, struct addrinfo*)`
- `close()`

### UvBufStore — `src/UvBufStore.h`

스트림과 UDP가 내부에서 사용하는 풀 기반 읽기/쓰기 버퍼 할당기.

- `alloc(dsize) -> char*`, `recycle(char*)`,
  `setMaxBufSize(cap)` (기본 10 MiB)

### HandleSender / HandleReceiver — `src/HandleSender.h`, `src/HandleReceiver.h`

IPC 파이프를 통한 파일 디스크립터 전달.

송신자 (클라이언트):

- `connect(path)`, `close()`, `sendHandle(UvStream*)`

수신자 (서버):

- `open(pipename, Lis)` — `Lis = fn(UvPipe*, uv_handle_type)`
- `close()` — Linux에서는 소켓 경로도 함께 unlink

## 10. 로깅과 유틸리티

- `vuv::UvStrErr(int err)` (`src/etc.h`) — 정적 문자열 누수가 없는
  `uv_strerror` 래퍼. 공유 정적 버퍼를 사용하므로 재진입 불가.
- 로그 레벨: `VERBOSE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `SILENT`
  (`src/logdef.h`). 매크로 `ule/ulw/uli/uld/ulv(FMT, ...)`는
  타임스탬프와 `file:line`을 함께 출력.
- `SetLogLevel(int)` / `UVC_SET_LOG_LEVEL(L)`로 상세도 조절.
  컴파일 타임 `UVCLOG_LEVEL` 기본값은 `VERBOSE`.

## 11. 스레딩

- 스레드당 하나의 `Loop` (`thread_local`). `UvAsync::send()`를 통하는
  경우를 제외하고 핸들을 스레드 간에 공유하지 마세요.
- `UvStrErr()`는 정적 버퍼를 공유하므로 스레드 안전하지 않습니다.
- `Immediate`는 루프 스레드 전용입니다. 워커 스레드에서는 `UvAsync`로
  루프를 깨운 뒤 지연 작업을 스케줄하세요.

## 12. 예제

7절의 TCP 에코 + 타이머 예제 외에:

타이머:

```cpp
vuv::UvTimer t;
t.init();
t.setOn([] { /* ... */ });
t.start(100, 1000);
t.stop();
t.close();
```

스레드 간 깨우기:

```cpp
vuv::UvAsync a;
a.init();
a.setOn([] { /* 루프 스레드에서 실행 */ });
// 어떤 스레드에서든:
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

프로세스 실행:

```cpp
vuv::UvSpawn sp;
sp.setOnConsoleOut([](auto b, auto n) {}, [](auto b, auto n) {});
sp.setOnExit([](int code) {});
sp.run({"/bin/echo", "hello"});
```

## 13. 패키징

CPack이 설정되어 있습니다. 배포용 아카이브를 만들려면 (설치 규칙을
통해 이 매뉴얼도 함께 포함됨):

```sh
cd build
cpack
```

`vuv-0.9.0-<system>.tar.gz` 및 `.zip` 아카이브가 생성됩니다.

## 14. 라이선스와 기여

MIT 라이선스 하에 배포됩니다. 자세한 내용은 `LICENSE`를 참고하세요.

기여를 환영합니다. `CONTRIBUTING.md`를 참고하세요.
