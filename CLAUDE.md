# Project Instructions for AI Agents

## Tech Stack
* Language: C++20
* UI Framework: FTXUI v6.1.9 (Functional Terminal (X) User Interface)
* Testing Framework: Catch2 v3.5.2
* Build System: CMake (with Ninja generator recommended)

## Important Commands
* **Configure:** `cmake -S . -B build -G Ninja`
* **Build:** `cmake --build build`
* **Run App:** `./build/calc_cli`
* **Run Tests:** `./build/run_tests`
* **Clang-format check:** `clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp tests/**/*.cpp`

## Architecture Notes
* The project follows a **Model-View-Controller (MVC)** architecture split into subdirectories under `src/`:
  * **`model/`**: Business logic, data repositories, and state.
    * `Calculator` (`src/model/calculator.hpp` / `src/model/calculator.cpp`): Pure infix expression evaluator.
    * `HistoryRepository` (`src/model/history_repository.hpp` / `src/model/history_repository.cpp`): Database repo for calculation history.
    * `ExchangeRate` (`src/model/exchange_rate.hpp` / `src/model/exchange_rate.cpp`): SQLite backend caching exchange rates.
    * `AppState` (`src/model/app_state.hpp`): Shared data structure containing UI state.
  * **`controller/`**: Handles communication between View and Model.
    * `AppController`, `HistoryController`, `ExchangeRateController`.
  * **`view/`**: User interface components.
    * `App` (`src/view/app.hpp` / `src/view/app.cpp`): FTXUI component tree root.
    * `CustomExchange` (`src/view/custom_exchange.hpp` / `src/view/custom_exchange.cpp`): Modal for adding custom exchange rates.
  * **`util/`**: Helpers, formatting, and constants.
    * `constants.hpp` (`src/util/constants.hpp`): Centralized constants file using `calc_cli` namespace.
    * `formatting.cpp` (`src/util/formatting.cpp`): Formatting functions like spacing and rounding.
* SQL string construction in `model/` uses `std::format` (requires C++20).

## Adding New Features / Modifying
* **Constants**: All magic numbers/strings (paths, TTL bounds, DB schemas) MUST be extracted to `src/util/constants.hpp` instead of hardcoding.
* **New operations**: Extend the parser logic in `src/model/parser.cpp`.
* **Tests**: Organized under `tests/` in matching subdirectories:
  * `tests/util/` (e.g. `test_constants.cpp`)
  * `tests/model/`
  * `tests/controller/`
  * `tests/view/`

## General Guidelines
* The [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) is the formatting authority. **Exception:** use 4-space indentation instead of 2.
* Private member variables use a **trailing underscore** (e.g., `state_`, `db_`).
* `using namespace ftxui;` is permitted in `.cpp` files only — never in headers.
* `#pragma once` is used for all header guards.
