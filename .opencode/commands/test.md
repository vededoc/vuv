---
description: Build and run available tests
---

Clean any stale GoogleTest checkout, configure tests, build, and run CTest:

!`rm -rf build/_deps/googletest-src build/_deps/googletest-build build/_deps/googletest-subbuild && cmake -S . -B build -DVUV_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure`
