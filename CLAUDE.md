# Project Instructions for AI Agents

## Tech Stack
* Language: C++17
* UI Framework: FTXUI v6.1.9 (Functional Terminal (X) User Interface)
* Testing Framework: Catch2 v3.5.2
* Build System: CMake (with Ninja generator recommended)

## Important Commands
* **Configure:** `cmake -S . -B build -G Ninja`
* **Build:** `cmake --build build`
* **Run App:** `./build/calc_cli`
* **Run Tests:** `./build/run_tests`
* **Clang-format check:** `clang-format --dry-run --Werror src/*.cpp src/*.hpp tests/*.cpp`

## Architecture Notes
* **`Calculator`** (`src/calculator.hpp` / `src/calculator.cpp`): Pure infix expression
  evaluator using a recursive descent parser. Has NO FTXUI dependency. Link via `calc_lib`.
* **`App`** (`src/app.hpp` / `src/app.cpp`): Wraps the entire FTXUI component tree.
  Accepts `AppState&` and `Calculator&` by reference. `GetComponent()` returns the root
  component for use in tests or the main loop.
* **`AppState`**: Plain data struct. Both the UI and tests read/write this directly.
  Modal visibility flags (`show_version_modal`, `show_history_modal`) live here so tests
  can assert them without access to private App members.
* Sub-menus (File, Edit, Help) use **Horizontal Animated** style. In tests, navigate
  sub-menu items with `Event::ArrowRight` / `Event::ArrowLeft`.
* The expression input uses a `CatchEvent` wrapper to intercept `Event::Return`
  (evaluate) and `Event::Escape` (clear) before they reach the Input component.

## Adding New Features
* New menu items: update the `MenuIndex` enum in `src/app.hpp` — never use hardcoded indices.
* New calculator operations: extend `Calculator::ParseAndEvaluate` in `src/calculator.cpp`.
* New tests: add `TEST_CASE` / `SECTION` blocks in `tests/test_main.cpp`.

## General Guidelines
* The [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) is the
  formatting authority. **Exception:** use 4-space indentation instead of 2.
* Private member variables use a **trailing underscore** (e.g., `state_`, `calc_`).
* `using namespace ftxui;` is permitted in `.cpp` files only — never in headers.
* `#pragma once` is used for all header guards.
