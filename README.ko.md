# vuv

![CI](https://github.com/vededoc/vuv/actions/workflows/ci.yml/badge.svg)

[English](README.md) | **한국어**

[libuv](https://libuv.org/) 의 C++20 래퍼 라이브러리입니다.

`vuv` 는 libuv 의 이벤트 루프, 핸들, 요청을 작고 사용하기 쉬운 C++ API 로 노출하면서, 필요할 때는 기저의 `uv_*` 타입에 직접 접근할 수 있도록 합니다.

## 주요 기능

- 주요 libuv 프리미티브에 대한 래퍼: `Loop`, `UvTcp`, `UvUdp`, `UvTimer`,
  `UvCheck`, `UvPrepare`, `UvPoll`, `UvSignal`, `UvPipe`, `UvTty`,
  `UvStream`, `UvAsync`, `UvIdle`, `Immediate`
- DNS 헬퍼: `DNSResolver`, `UvGetAddrInfo`
- `UvSpawn` 을 통한 프로세스 실행
- 유틸리티 헬퍼: `UvBufStore`, `HandleSender` / `HandleReceiver`
- CMake 패키지 설정 (`find_package(vuv)`) 및 설치 규칙

## 요구 사항

- C++20 호환 컴파일러 (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16+
- libuv 개발 헤더 및 라이브러리

Debian/Ubuntu:

```sh
sudo apt-get install -y libuv1-dev cmake g++
```

## 빌드

```sh
cmake -S . -B build -DVUV_BUILD_TESTS=ON
cmake --build build
```

## 테스트 실행

```sh
ctest --test-dir build --output-on-failure
```

## 설치

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

## 다른 프로젝트에서 사용

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

## 패키징

CPack 이 설정되어 있습니다. 배포용 아카이브를 만들려면:

```sh
cd build
cpack
```

`vuv-0.9.0-<system>.tar.gz` 및 `.zip` 아카이브가 생성됩니다.

## 예제

포함된 예제(TCP 클라이언트/서버 + 타이머)를 빌드하고 실행하려면:

```sh
cmake -S . -B build -DVUV_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/vuv_tcp_echo_timer
```

## 라이선스

MIT 라이선스 하에 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 를 참고하세요.

## 기여

기여를 환영합니다. [CONTRIBUTING.md](CONTRIBUTING.md) 를 참고하세요.
