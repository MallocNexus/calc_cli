# calc-cli Implementation Plan

A terminal-based calculator TUI built with **FTXUI**, following the **Google C++ Style Guide**
and a clean **MVC layer separation** with explicit `model/`, `view/`, and `controller/`
subdirectories inside `src/`.

---

## 1. Goals

- Provide a fully interactive infix expression calculator in the terminal.
- Enforce a strict three-layer MVC architecture where each layer has a clear, single
  responsibility and enforced dependency direction: View → Controller → Model.
- Every layer independently testable: Model with zero FTXUI, Controller with zero FTXUI,
  View testable headlessly via `GetComponent()`.
- Conform strictly to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
  with the project-local exception of **4-space indentation**.

---

## 2. Folder Structure

```
calc-cli/
├── CMakeLists.txt                    # Root build definition — 3 library targets
├── .clang-format                     # Google style, 4-space indent
├── .gitignore
├── CLAUDE.md                         # AI agent instructions
├── README.md                         # Project overview
├── calc-cli-impl-plan.md             # This file
│
├── src/
│   ├── main.cpp                      # Entry point only — wires all three layers
│   │
│   ├── model/                        # Layer 1: Pure data and business logic
│   │   ├── app_state.hpp             # AppState struct + MenuIndex enum (plain data)
│   │   ├── calculator.hpp            # EvaluationResult, Calculator declaration
│   │   └── calculator.cpp            # Recursive descent infix parser
│   │
│   ├── controller/                   # Layer 2: Event handling and state mutation
│   │   ├── app_controller.hpp        # AppController class declaration
│   │   └── app_controller.cpp        # Connects view events -> model -> state updates
│   │
│   └── view/                         # Layer 3: FTXUI rendering only
│       ├── app.hpp                   # App class declaration
│       └── app.cpp                   # Builds FTXUI component tree, calls controller
│
└── tests/
    └── test_main.cpp                 # Catch2 unit + integration tests
```

### Dependency Direction (enforced by CMake targets)

```
main.cpp
    │
    ▼
view/app  ──────────────► controller/app_controller
(FTXUI)                         │
                                ▼
                         model/calculator + model/app_state
                         (no FTXUI, pure C++)
```

- **`model/`** knows nothing about the controller or view.
- **`controller/`** knows the model, knows nothing about FTXUI.
- **`view/`** knows FTXUI and the controller; reads model state via `AppState&`.
- **`main.cpp`** is the only place all three are wired together.

---

## 3. Architecture

### 3.1 Model Layer (`src/model/`)

#### `app_state.hpp` — shared plain-data struct

Extracted from `app.hpp` into its own file so it can be included by all three layers
without pulling in FTXUI or controller headers:

```cpp
struct AppState {
    std::string expression_input;
    int         cursor_position    = 0;
    std::string result_display;       // "= 7" on success, "Error: ..." on failure
    bool        error_state        = false;
    bool        show_version_modal = false;
    bool        show_history_modal = false;
};
```

#### `calculator.hpp` / `calculator.cpp` — pure logic, no FTXUI

```cpp
struct EvaluationResult {
    bool        ok;
    double      value;
    std::string error;
};

class Calculator {
  public:
    EvaluationResult Evaluate(const std::string& expression);
    void             ClearHistory();
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;
  private:
    std::vector<std::pair<std::string, std::string>> history_;
    EvaluationResult ParseAndEvaluate(const std::string& expr);
};
```

#### `MenuIndex` enum — also lives in `app_state.hpp`

Named indices for sub-menu items. Kept in the model layer because they represent
domain-level intent (e.g. FILE_QUIT), not view layout:

```cpp
enum MenuIndex {
    FILE_QUIT    = 0,
    EDIT_CLEAR   = 0,
    EDIT_HISTORY = 1,
    HELP_VERSION = 0,
};
```

---

### 3.2 Controller Layer (`src/controller/`)

#### `app_controller.hpp` / `app_controller.cpp`

The controller owns all business logic that was previously scattered through lambdas
inside `app.cpp`. It has **zero FTXUI dependency** — it only reads/writes `AppState`
and calls `Calculator`:

```cpp
class AppController {
  public:
    AppController(AppState& state, Calculator& calc, std::function<void()> on_quit);

    // Called by the view in response to user events:
    void OnEvaluate();       // Return pressed in input — calls calc, updates state
    void OnClear();          // Edit -> Clear
    void OnQuit();           // File -> Quit
    void OnOpenHistory();    // Edit -> History
    void OnCloseHistory();   // History modal Close button
    void OnOpenVersion();    // Help -> Version
    void OnCloseVersion();   // Version modal Close button

    // Read-only access to history for the view to render:
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    AppState&              state_;
    Calculator&            calc_;
    std::function<void()>  on_quit_;

    std::string FormatResult(double value) const;
};
```

> **Key benefit:** `AppController` can be unit-tested entirely without FTXUI.
> Tests instantiate it, call `OnEvaluate()`, and assert `state_.result_display`.

---

### 3.3 View Layer (`src/view/`)

#### `app.hpp` / `app.cpp` — pure FTXUI layout and rendering

The `App` class becomes a true view: it builds the component tree and wires user
events to `AppController` methods. It contains **no `if/else` business logic**:

```cpp
class App {
  public:
    App(AppState& state, AppController& controller);
    ftxui::Component GetComponent();

  private:
    AppState&       state_;
    AppController&  controller_;

    std::vector<std::string> top_menu_entries_ = {"File", "Edit", "Help"};
    int top_menu_selected_ = 0;

    std::vector<std::string> file_entries_ = {"Quit"};
    std::vector<std::string> edit_entries_ = {"Clear", "History"};
    std::vector<std::string> help_entries_ = {"Version"};
    int file_selected_ = 0, edit_selected_ = 0, help_selected_ = 0;

    ftxui::Component component_;
};
```

A concrete example of what moves OUT of the view's event lambdas:

```cpp
// BEFORE (app.cpp — business logic in the view):
if (event == Event::Return) {
    EvaluationResult res = calc_.Evaluate(state_.expression_input);
    if (res.ok) {
        state_.result_display = "= " + FormatResult(res.value);
        state_.error_state = false;
    } else { ... }
    return true;
}

// AFTER (app.cpp — view delegates to controller):
if (event == Event::Return) {
    controller_.OnEvaluate();
    return true;
}
```

---

### 3.4 Entry Point (`src/main.cpp`)

The only file that includes all three layers:

```cpp
int main() {
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    AppState      state;
    Calculator    calc;
    AppController controller(state, calc, [&screen] { screen.ExitLoopClosure()(); });
    App           app(state, controller);

    screen.Loop(app.GetComponent());
    return EXIT_SUCCESS;
}
```

---

## 4. UI Layout

```
┌──────────────────────────────────────────┐
│  File │ Edit │ Help                       │  <- Horizontal animated top menu (view)
├──────────────────────────────────────────┤
│  Quit                                    │  <- Active sub-menu (view)
├──────────────────────────────────────────┤
│  > 3 + 4 * (2 - 1)_                     │  <- Expression Input (view, green when focused)
├──────────────────────────────────────────┤
│  = 7                                     │  <- Result display (green=ok, red=error)
└──────────────────────────────────────────┘
```

Event flow: **View captures keystroke → calls Controller method → Controller updates
AppState → View re-renders from AppState**

---

## 5. CMakeLists.txt Plan

Three library targets enforce the dependency direction at the build-system level:

```cmake
cmake_minimum_required(VERSION 3.14)
project(CalcCli VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# FTXUI + Catch2 (via FetchContent — unchanged)

# --- Layer 1: Model (no FTXUI) ---
add_library(calc_lib src/model/calculator.cpp)
target_include_directories(calc_lib PUBLIC src/)
# Headers accessed as: #include "model/calculator.hpp"
#                       #include "model/app_state.hpp"

# --- Layer 2: Controller (no FTXUI) ---
add_library(controller_lib src/controller/app_controller.cpp)
target_include_directories(controller_lib PUBLIC src/)
target_link_libraries(controller_lib PUBLIC calc_lib)

# --- Layer 3: View (FTXUI) ---
add_library(app_lib src/view/app.cpp)
target_include_directories(app_lib PUBLIC src/)
target_link_libraries(app_lib PUBLIC
    ftxui::screen ftxui::dom ftxui::component
    controller_lib
)

# Main executable
add_executable(calc_cli src/main.cpp)
target_link_libraries(calc_cli PRIVATE app_lib)

# Test executable — can link any layer independently
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE
    Catch2::Catch2WithMain app_lib controller_lib calc_lib)
```

> **Why three targets?** CMake enforces the dependency graph at link time.
> `controller_lib` physically cannot link against FTXUI because it doesn't declare that
> dependency. This prevents accidental cross-layer coupling from compiling silently.

---

## 6. Test Strategy (`tests/test_main.cpp`)

### Unit Tests — Model (link: `calc_lib` only)

| Test | Input | Expected |
|------|-------|----------|
| Basic addition | `"2 + 3"` | `ok=true, value=5` |
| Operator precedence | `"2 + 3 * 4"` | `ok=true, value=14` |
| Parentheses | `"(2 + 3) * 4"` | `ok=true, value=20` |
| Division by zero | `"5 / 0"` | `ok=false` |
| Invalid input | `"abc"` | `ok=false` |
| Negative numbers | `"-3 + 1"` | `ok=true, value=-2` |
| Floating point | `"1.5 + 2.5"` | `ok=true, value=4.0` |
| History tracking | Evaluate twice | `GetHistory().size() == 2` |
| Clear history | After clear | `GetHistory().empty()` |

### Unit Tests — Controller (link: `controller_lib` only, no FTXUI)

| Test | Action | Assertion |
|------|--------|-----------|
| OnEvaluate success | `state.expression_input = "3+4"`, call `OnEvaluate()` | `state.result_display == "= 7"` |
| OnEvaluate error | `state.expression_input = "1/0"`, call `OnEvaluate()` | `state.error_state == true` |
| OnClear | After evaluate, call `OnClear()` | `state.expression_input.empty()` |
| OnOpenHistory | Call `OnOpenHistory()` | `state.show_history_modal == true` |
| OnCloseHistory | After open, call `OnCloseHistory()` | `state.show_history_modal == false` |
| OnOpenVersion | Call `OnOpenVersion()` | `state.show_version_modal == true` |
| OnQuit | Call `OnQuit()` | `quit_called == true` |

### Integration Tests — View (link: `app_lib`, headless event simulation)

| Test | Events | Assertion |
|------|--------|-----------|
| Evaluate via keypress | navigate to input, `Return` | `result_display == "= 7"` |
| Error via keypress | navigate to input, bad expr, `Return` | `error_state == true` |
| Escape clears | navigate to input, `Escape` | `expression_input.empty()` |
| Edit -> Clear | navigate menu, `Return` | `expression_input.empty()` |
| File -> Quit | navigate menu, `Return` | `quit_called == true` |
| History modal | Edit -> History, `Return` | `show_history_modal == true` |
| Version modal | Help -> Version, `Return` | `show_version_modal == true` |

---

## 7. `.clang-format` Config

```yaml
BasedOnStyle:   Google
IndentWidth:    4
ColumnLimit:    100
```

---

## 8. Implementation Phases

### Phase 1 — Scaffold ✅ Done
- [x] `CMakeLists.txt`
- [x] `.clang-format`, `.gitignore`, `CLAUDE.md`, `README.md`

### Phase 2 — Model Layer ✅ Done (needs move)
- [x] `src/calculator.hpp` → move to `src/model/calculator.hpp`
- [x] `src/calculator.cpp` → move to `src/model/calculator.cpp`
- [ ] Extract `src/model/app_state.hpp` from `src/app.hpp`

### Phase 3 — Controller Layer 🆕 New
- [ ] `src/controller/app_controller.hpp`
- [ ] `src/controller/app_controller.cpp`
  - `OnEvaluate()`: call `calc_.Evaluate()`, format, write to `state_`
  - `OnClear()`: clear `expression_input`, `result_display`, `error_state`
  - `OnQuit()`: call `on_quit_()`
  - `OnOpenHistory()` / `OnCloseHistory()`: toggle `show_history_modal`
  - `OnOpenVersion()` / `OnCloseVersion()`: toggle `show_version_modal`

### Phase 4 — View Layer (refactor)
- [x] `src/app.hpp` → move to `src/view/app.hpp`, update to take `AppController&`
- [x] `src/app.cpp` → move to `src/view/app.cpp`, strip business logic, delegate to controller
- [x] `src/main.cpp` → update includes + wire `AppController`

### Phase 5 — Update CMakeLists.txt
- [ ] Add `controller_lib` target
- [ ] Update source paths to `src/model/`, `src/view/`, `src/controller/`
- [ ] Remove old flat `src/` targets

### Phase 6 — Tests
- [ ] Add controller unit tests (no FTXUI needed)
- [ ] Update integration test includes to new header paths

### Phase 7 — Polish
- [ ] Clean build: `cmake -S . -B build -G Ninja && cmake --build build`
- [ ] All tests pass: `./build/run_tests`
- [ ] Clang-format: `clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp tests/*.cpp`

---

## 9. Key Conventions (Google Style + Project Local)

| Rule | Value |
|------|-------|
| Indentation | 4 spaces (project override) |
| Private members | Trailing underscore (`state_`, `calc_`, `controller_`) |
| Header guards | `#pragma once` |
| Include paths | Relative to `src/` root: `"model/calculator.hpp"`, `"view/app.hpp"` |
| Includes order | Related header → C std → C++ std → third-party |
| Type/function naming | `PascalCase` |
| Variable/parameter naming | `snake_case` |
| Line length | <= 100 chars |
| Namespaces | `using namespace ftxui;` in `.cpp` files only, never in headers |

---

## 10. What Changes vs. Original Plan

| Item | Before | After |
|------|--------|-------|
| `calculator.hpp/cpp` | `src/` | `src/model/` |
| `app_state.hpp` | embedded in `src/app.hpp` | `src/model/app_state.hpp` (own file) |
| `app.hpp/cpp` | `src/` | `src/view/` |
| Business logic in App | lambdas inside `app.cpp` constructor | `AppController` methods |
| `app.cpp` constructor | ~165 lines, mixed concerns | pure layout/render, delegates all logic |
| CMake targets | `calc_lib`, `app_lib` | `calc_lib`, `controller_lib`, `app_lib` |
| Controller unit tests | none | 7 new tests, zero FTXUI dependency |
| Main includes | `app.hpp`, `calculator.hpp` | `model/app_state.hpp`, `model/calculator.hpp`, `controller/app_controller.hpp`, `view/app.hpp` |
