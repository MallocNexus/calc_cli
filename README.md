# calc-cli

A terminal-based infix expression calculator built in C++ using
[FTXUI](https://github.com/ArthurSonzogni/FTXUI).

## Overview

`calc-cli` demonstrates a clean two-layer architecture for FTXUI applications:

* **Logic layer** (`Calculator`): A pure C++ recursive descent parser that evaluates infix
  arithmetic expressions. It has zero FTXUI dependency and is independently unit-testable.
* **UI layer** (`App`): Wraps the entire FTXUI component tree. Accepts the state and calculator
  by reference, making it fully testable headlessly via `GetComponent()`.

### Features

* Infix arithmetic: `+`, `-`, `*`, `/`, parentheses, unary minus, floating-point numbers.
* Horizontal animated menus: File, Edit, Help.
* History modal (Edit → History): browse past evaluations.
* Version modal (Help → Version).
* Escape to clear the current expression.
* Result displayed in green (success) or red (error).
* 100% testable architecture — Catch2 tests simulate keyboard events without launching
  a terminal.

## Requirements

* C++17 compatible compiler (Apple Clang, GCC, Clang)
* CMake >= 3.14
* Ninja (optional, but recommended)

*FTXUI and Catch2 are automatically fetched via CMake `FetchContent`.*

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run

```bash
./build/calc_cli
```

### Key bindings

| Key | Action |
|-----|--------|
| Type expression | Enters characters into the input field |
| `Return` | Evaluates the expression |
| `Escape` | Clears the input and result |
| `ArrowUp` / `ArrowDown` | Move between menu bar and sub-menus |
| `ArrowRight` / `ArrowLeft` | Navigate sub-menu items |
| `Tab` | Move focus to the next component |

## Tests

```bash
./build/run_tests
```

Tests cover:

* Calculator unit tests (logic only, no FTXUI)
* App integration tests (headless keyboard event simulation)

## Appendix

### FTXUI

[FTXUI](https://github.com/ArthurSonzogni/FTXUI) is a modern, declarative C++ library for
building terminal UIs. It handles all rendering, keyboard input, and terminal escape sequences.

### Catch2

[Catch2](https://github.com/catchorg/Catch2) drives integration tests by instantiating the
`App` headlessly and sending `ftxui::Event` objects to simulate real user input.
