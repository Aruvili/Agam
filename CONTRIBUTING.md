# Contributing to Agam

Thank you for your interest in contributing to the Agam programming language!

## Getting Started

1. **Fork** the repository
2. **Clone** your fork: `git clone https://github.com/<you>/agam.git`
3. **Create a branch**: `git checkout -b feature/my-feature`
4. **Build** the project (see [README.md](README.md))

## Development Requirements

- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.20+
- LLVM 17+
- Flex 2.6+
- Bison 3.8+

## Code Style

- Run `clang-format` before committing (config in `.clang-format`)
- Follow the LLVM coding style
- Use meaningful variable/function names
- Add comments for non-obvious logic

## Submitting Changes

1. Ensure all tests pass: `cd build && ctest --output-on-failure`
2. Add tests for new features
3. Write clear commit messages following [Conventional Commits](https://www.conventionalcommits.org/)
4. Open a Pull Request with a description of your changes

## Commit Message Format

```
<type>(<scope>): <description>

[optional body]
```

Types: `feat`, `fix`, `docs`, `test`, `refactor`, `build`, `ci`

Examples:
- `feat(lexer): add string literal support`
- `fix(parser): resolve shift-reduce conflict in if-else`
- `test(semantic): add type checker edge cases`

## Reporting Issues

- Use GitHub Issues
- Include: steps to reproduce, expected vs actual behavior, Agam source code if applicable

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
