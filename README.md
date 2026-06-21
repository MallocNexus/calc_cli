# calc-cli

A terminal-based infix expression calculator with real-time currency exchange rates, built in C++ using [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

## Architecture

`calc-cli` follows a decoupled, layered architecture to maintain a strict separation of concerns:

* **View Layer ([src/view/](file:///Users/mattswart/Source/CPP/calc-cli/src/view/))**: Builds the FTXUI component tree ([app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp) and [custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.cpp)). It contains no business logic and binds reactively to the application state via `ftxui::Ref` references.
* **Controller Layer ([src/controller/](file:///Users/mattswart/Source/CPP/calc-cli/src/controller/))**: Routes user events, coordinates state modifications, and manages lifecycle workflows ([app_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/controller/app_controller.cpp), [history_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/controller/history_controller.cpp), and [exchange_rate_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/controller/exchange_rate_controller.cpp)).
* **Service Layer ([src/service/](file:///Users/mattswart/Source/CPP/calc-cli/src/service/))**: Houses stateless domain logic ([calculator.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/service/calculator.cpp) and [parser.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/service/parser.cpp)) to evaluate infix math expressions. It decouples currency rate resolution via a callback interface.
* **Model Layer ([src/model/](file:///Users/mattswart/Source/CPP/calc-cli/src/model/))**: Centralizes the flat application state ([app_state.hpp](file:///Users/mattswart/Source/CPP/calc-cli/src/model/app_state.hpp)) and encapsulates SQLite database storage for history entries ([history_repository.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/model/history_repository.cpp)) and exchange rate cache ([exchange_rate.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/model/exchange_rate.cpp)).

For details, see [separation-of-concerns.md](file:///Users/mattswart/Source/CPP/calc-cli/docs/separation-of-concerns.md).

### Features

* **Advanced Infix Arithmetic**: Supports `+`, `-`, `*`, `/`, parentheses, unary minus, and floating-point numbers.
* **Currency Rate Conversions**: Evaluates exchange statements like `exchange(AUD, USD)`. Queries are resolved using a local SQLite cache with fallback downloading from the Frankfurter API via the **CPR (C++ Requests)** library.
* **Inline Calculation History**: Browse, select, and recall past evaluations directly in the focusable, scrollable list panel at the bottom of the screen.
* **Animated Tabs & Modals**: Smoothly navigate File, Edit, Exchange, and Help menus. Add custom rate mappings via the Custom Exchange Rate modal dialog.
* **100% Testable**: Built with Catch2, the test suite headlessly simulates keyboard actions and ANSI rendering checks without spinning up a live terminal.

## Requirements

* C++20 compatible compiler (Apple Clang, GCC, Clang)
* SQLite3 library installed
* CMake >= 3.14
* Ninja (optional, but recommended)

*FTXUI, CPR, nlohmann/json, and Catch2 are automatically fetched via CMake `FetchContent`.*

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run

`calc-cli` supports two execution modes: **Interactive TUI Mode** and **Headless CLI Mode**.

### 1. Interactive TUI Mode

To launch the fullscreen interactive terminal interface, run the executable without any arguments:

```bash
./build/calc_cli
```

### 2. Headless CLI Mode

Evaluate expressions directly from the terminal or pipe them into the application:

* **Direct expression evaluation**:
  ```bash
  ./build/calc_cli "2 + 2 * (10 / 5)"
  ```
* **Explicit expression evaluation**:
  ```bash
  ./build/calc_cli --expr "3 + 4"
  ```
* **Print only the result value** (useful for scripts):
  ```bash
  ./build/calc_cli --expr "3 + 4" --value
  ```
* **Evaluation with exchange rates**:
  ```bash
  ./build/calc_cli --expr "100" --exchange "AUD,USD"
  ```
* **Read expressions from a file**:
  ```bash
  ./build/calc_cli --stdin-file path/to/file.txt
  ```
* **Pipe input into the calculator**:
  ```bash
  echo "100 + 50" | ./build/calc_cli
  ```

---

### Key Bindings

| Component / Area | Key | Action |
|---|---|---|
| **Expression Input** | Letters / Numbers | Enters characters into the input line |
| | `Return` | Evaluates the expression |
| | `Escape` | Clears the input and result strings |
| **Menus (File/Edit/Exchange/Help)** | `ArrowLeft` / `ArrowRight` | Navigate between top-level tabs |
| | `ArrowDown` | Focus into a tab's sub-menu |
| | `Return` | Triggers the selected option |
| **History Panel** | `ArrowUp` / `ArrowDown` | Navigate up/down through history items |
| | `Return` | Recalls the selected expression to the input field |
| **Modals** | `Escape` | Closes active modals (Version, Custom Exchange) |
| **Global TUI** | `Tab` | Shift keyboard focus to the next component |

---

## Tests

To run the full unit and integration test suite:

```bash
./build/run_tests
```

Tests cover:
* **Service unit tests** ([test_calculator.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_calculator.cpp), [test_parser.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_parser.cpp)) in complete isolation.
* **Model unit tests** ([test_history_repository.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_history_repository.cpp), [test_exchange_rate.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_exchange_rate.cpp)).
* **Controller unit tests** ([test_app_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_app_controller.cpp), [test_history_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_history_controller.cpp), [test_exchange_rate_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_exchange_rate_controller.cpp)).
* **View integration tests** ([test_app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_app.cpp), [test_custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_custom_exchange.cpp)) simulating headless keyboard actions.

## Documentation Index

Detailed design specifications, patterns, and planning logs are available under the [docs/](file:///Users/mattswart/Source/CPP/calc-cli/docs/) directory:

* [Separation of Concerns](file:///Users/mattswart/Source/CPP/calc-cli/docs/separation-of-concerns.md): Domain layers details and component architecture diagram.
* [App UI Specifications](file:///Users/mattswart/Source/CPP/calc-cli/docs/app-ui-specs.md): Colors, styles, dimensions, and layout details of components.
* [Data Binding Patterns](file:///Users/mattswart/Source/CPP/calc-cli/docs/data-binding-patterns.md): FTXUI reactive bindings, state ownership, and callbacks.
* [FTXUI Event Flow](file:///Users/mattswart/Source/CPP/calc-cli/docs/ftxui-event-flow.md): Sequence diagram of how keyboard events propagate to controllers and views.
* [Future Feature Ideas](file:///Users/mattswart/Source/CPP/calc-cli/docs/planning/ideas/future-features-ideas.md): Proposals for sorting history, exchange shorthands, F1 context help, and navigation.
