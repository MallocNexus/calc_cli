# calc-cli Implementation Plan

A terminal-based calculator TUI built with **FTXUI**, following the **Google C++ Style Guide**
and the architectural patterns established in the `FTXUI-HelloWorld` reference project.

---

## 1. Goals

- Provide a fully interactive infix expression calculator in the terminal.
- Decouple pure calculation logic from the UI so both can be unit-tested independently.
- Mirror the testable-architecture pattern from `FTXUI-HelloWorld`: headless `App` class,
  `AppState` struct, `GetComponent()` accessor.
- Conform strictly to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
  with the project-local exception of **4-space indentation**.

---

## 2. Folder Structure

```
calc-cli/
├── CMakeLists.txt              # Root build definition
├── .clang-format               # Clang-format config (Google style, 4-space indent)
├── .gitignore
├── CLAUDE.md                   # AI agent instructions (mirrors HelloWorld pattern)
├── README.md                   # Human-readable project overview
├── calc-cli-impl-plan.md       # This file
│
├── src/
│   ├── main.cpp                # Entry point only — wires screen + App, nothing else
│   ├── app.hpp                 # AppState struct, MenuIndex enum, App class declaration
│   ├── app.cpp                 # App constructor: builds full FTXUI component tree
│   ├── calculator.hpp          # Calculator class declaration (pure logic, no FTXUI)
│   └── calculator.cpp          # Calculator class implementation
│
└── tests/
    └── test_main.cpp           # Catch2 integration + unit tests
```

### Rationale for `src/` subdirectory
The HelloWorld project keeps all sources at the root, which works for a small scaffold.
`calc-cli` introduces two distinct modules (`App` UI layer and `Calculator` logic layer),
so separating them into `src/` keeps the root clean and maps naturally to CMake targets.

---

## 3. Architecture

### 3.1 Layered Design

```
┌─────────────────────────────────┐
│           main.cpp              │  Wires ScreenInteractive + App only
├─────────────────────────────────┤
│   App  (app.hpp / app.cpp)      │  FTXUI component tree, event routing
│   AppState                      │  Plain data struct shared across UI
├─────────────────────────────────┤
│  Calculator (calculator.hpp/cpp)│  Pure math logic — NO FTXUI dependency
│  EvaluationResult               │  Value-type result (value or error string)
└─────────────────────────────────┘
```

### 3.2 `AppState` struct (`src/app.hpp`)

Holds all mutable state that the UI reads and writes:

```cpp
struct AppState {
    std::string expression_input;      // Current expression string
    int         cursor_position = 0;
    std::string result_display;        // Formatted result or error
    std::vector<std::string> history;  // Past "expression = result" strings
    int         history_selected = 0;
    bool        error_state = false;   // Styles result red on error
};
```

### 3.3 `MenuIndex` enum (`src/app.hpp`)

```cpp
enum MenuIndex {
    FILE_QUIT    = 0,
    EDIT_CLEAR   = 0,
    EDIT_HISTORY = 1,
    HELP_VERSION = 0,
};
```

### 3.4 `App` class (`src/app.hpp`)

```cpp
class App {
  public:
    App(AppState& state, Calculator& calc, std::function<void()> on_quit);
    ftxui::Component GetComponent();

  private:
    AppState&              state_;
    Calculator&            calc_;
    std::function<void()>  on_quit_;

    // Top-level menu
    std::vector<std::string> top_menu_entries_ = {"File", "Edit", "Help"};
    int top_menu_selected_ = 0;

    // Sub-menus
    std::vector<std::string> file_entries_  = {"Quit"};
    std::vector<std::string> edit_entries_  = {"Clear", "History"};
    std::vector<std::string> help_entries_  = {"Version"};
    int file_selected_ = 0, edit_selected_ = 0, help_selected_ = 0;

    // Modal flags
    bool show_version_modal_  = false;
    bool show_history_modal_  = false;

    ftxui::Component component_;
};
```

> **Note:** Trailing underscore on private members follows Google style.

### 3.5 `EvaluationResult` & `Calculator` class (`src/calculator.hpp`)

```cpp
struct EvaluationResult {
    bool        ok;
    double      value;      // valid when ok == true
    std::string error;      // valid when ok == false
};

class Calculator {
  public:
    EvaluationResult Evaluate(const std::string& expression);
    void             ClearHistory();
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    std::vector<std::pair<std::string, std::string>> history_;

    // Shunting-yard algorithm: tokenise -> parse -> evaluate
    EvaluationResult ParseAndEvaluate(const std::string& expr);
};
```

---

## 4. UI Layout

```
┌──────────────────────────────────────────┐
│  File │ Edit │ Help                       │  <- Horizontal animated top menu
├──────────────────────────────────────────┤
│  Quit                                    │  <- Active sub-menu (Tab container)
├──────────────────────────────────────────┤
│  > 3 + 4 * (2 - 1)_                     │  <- Expression Input (focused, green)
├──────────────────────────────────────────┤
│  = 7                                     │  <- Result display (green / red on err)
└──────────────────────────────────────────┘
```

- **Expression Input**: Single-line `Input` with `Return` key wired to `Calculator::Evaluate`.
- **Result Display**: `text(state.result_display)` coloured `Color::Green` on success,
  `Color::Red` on error.
- **History Modal**: Scrollable `Menu` of past evaluations, triggered from Edit -> History.
- **Version Modal**: Static text popup, triggered from Help -> Version.

### Key Event Bindings

| Key | Action |
|-----|--------|
| `Return` | Evaluate current expression |
| `Escape` in input | Clear input field |
| `ArrowUp` / `ArrowDown` | Enter / leave sub-menu |
| `ArrowRight` / `ArrowLeft` | Navigate sub-menu items |

---

## 5. CMakeLists.txt Plan

```cmake
cmake_minimum_required(VERSION 3.14)
project(CalcCli VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# FTXUI
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG        v6.1.9)
FetchContent_MakeAvailable(ftxui)

# Catch2
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2)
FetchContent_MakeAvailable(Catch2)

# Core logic library (NO FTXUI dependency — keeps it testable in isolation)
add_library(calc_lib src/calculator.cpp)
target_include_directories(calc_lib PUBLIC src/)

# App UI library (depends on FTXUI + calc_lib)
add_library(app_lib src/app.cpp)
target_include_directories(app_lib PUBLIC src/)
target_link_libraries(app_lib PUBLIC
    ftxui::screen ftxui::dom ftxui::component calc_lib)

# Main executable
add_executable(calc_cli src/main.cpp)
target_link_libraries(calc_cli PRIVATE app_lib)

# Test executable
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE Catch2::Catch2WithMain app_lib calc_lib)
```

> **Key difference from HelloWorld:** `calc_lib` (pure logic) is a *separate* CMake target with
> no FTXUI dependency. This means `tests/test_main.cpp` can link `calc_lib` alone to unit-test
> math without any UI overhead.

---

## 6. Test Strategy (`tests/test_main.cpp`)

### Unit Tests — Calculator Logic (link: `calc_lib` only)

| Test | Input | Expected |
|------|-------|----------|
| Basic addition | `"2 + 3"` | `ok=true, value=5` |
| Operator precedence | `"2 + 3 * 4"` | `ok=true, value=14` |
| Parentheses | `"(2 + 3) * 4"` | `ok=true, value=20` |
| Division by zero | `"5 / 0"` | `ok=false` |
| Invalid expression | `"abc"` | `ok=false` |
| Negative numbers | `"-3 + 1"` | `ok=true, value=-2` |
| Floating point | `"1.5 + 2.5"` | `ok=true, value=4.0` |
| History tracking | Evaluate twice | `GetHistory().size() == 2` |
| Clear history | After clear | `GetHistory().empty()` |

### Integration Tests — App UI (link: `app_lib` + simulate events)

| Test | Events | Assertion |
|------|--------|-----------|
| Type and evaluate | set `expression_input = "3 + 4"`, send `Return` | `result_display == "= 7"` |
| Error display | set `expression_input = "1 / 0"`, send `Return` | `error_state == true` |
| Edit -> Clear | navigate to Edit -> Clear, `Return` | `expression_input.empty()` |
| Quit | navigate to File -> Quit, `Return` | `quit_called == true` |
| History modal opens | navigate Edit -> History, `Return` | `show_history_modal == true` |
| Version modal opens | navigate Help -> Version, `Return` | `show_version_modal == true` |

---

## 7. `.clang-format` Config

```yaml
BasedOnStyle:   Google
IndentWidth:    4
ColumnLimit:    100
```

---

## 8. Implementation Phases

### Phase 1 — Scaffold
- [ ] `CMakeLists.txt`
- [ ] `.clang-format`, `.gitignore`, `CLAUDE.md`, `README.md`

### Phase 2 — Pure Logic Layer
- [ ] `src/calculator.hpp` — `EvaluationResult`, `Calculator` declaration
- [ ] `src/calculator.cpp` — infix parser using shunting-yard algorithm

### Phase 3 — UI Layer
- [ ] `src/app.hpp` — `AppState`, `MenuIndex`, `App` declaration
- [ ] `src/app.cpp` — full FTXUI component tree
- [ ] `src/main.cpp` — entry point only

### Phase 4 — Tests
- [ ] `tests/test_main.cpp` — unit + integration test cases

### Phase 5 — Polish
- [ ] Verify clean build: `cmake -S . -B build -G Ninja && cmake --build build`
- [ ] All tests pass: `./build/run_tests`
- [ ] Clang-format clean: `clang-format --dry-run --Werror src/*.cpp src/*.hpp tests/*.cpp`

---

## 9. Key Conventions (Google Style + Project Local)

| Rule | Value |
|------|-------|
| Indentation | 4 spaces (project override) |
| Private members | Trailing underscore (`state_`, `calc_`) |
| Header guards | `#pragma once` |
| Includes order | Related header -> C std -> C++ std -> third-party |
| Type/function naming | `PascalCase` |
| Variable/parameter naming | `snake_case` |
| Line length | <= 100 chars |
| Namespaces | `using namespace ftxui;` in `.cpp` files only, never in headers |
