# Contributing to vuv

Thanks for your interest in contributing.

## Development setup

```sh
cmake -S . -B build -DVUV_BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Guidelines

- Follow the existing code style (see `.clang-format`).
- Add or update tests in `tests/` for any behavior change.
- Keep public headers in `src/` and include them as `"..."`.
- Ensure the project builds with `-Werror=return-type` (no missing returns).
- Run `clang-format` on changed files before submitting.

## Pull requests

1. Fork the repository and create a feature branch.
2. Make your changes with tests.
3. Open a pull request against `main` with a clear description.

## Reporting issues

Please open a GitHub issue with reproduction steps and environment details.
