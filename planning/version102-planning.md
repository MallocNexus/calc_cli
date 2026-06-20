# Plan: Keyboard Navigation & Scrolling for Calculation History (v1.0.2)

## Overview

This plan details the implementation of keyboard navigation and scrolling for the calculation history area using the cursor **Up** and **Down** keys.

Following FTXUI's design patterns (as illustrated in their official `menu_in_frame.cpp` example), we will use an interactive **`ftxui::Menu`** wrapped inside a **`frame`** and **`vscroll_indicator`** (or the shortcut **`yframe`**) to achieve keyboard-scrollable history. 

Crucially, rather than hardcoding a static height limit, we will allow the history panel to expand dynamically to fill the available screen space at runtime using FTXUI's **`flex`** layout element.

---

## Design Selection: Focusable Menu in Frame with Dynamic Flex Layout

We will implement a focusable history list using the `ftxui::Menu` component. The menu's rendered element is decorated with `yframe` (`vscroll_indicator | frame`), and `flex`. This delegation allows the FTXUI framework to handle item selection, keyboard navigation (`ArrowUp`/`ArrowDown`), focus tracking, and scrolling out-of-the-box, while expanding dynamically to fit the terminal size.

### Key Components from FTXUI Example
1. **Dynamic Vector of Strings**: Maintain `std::vector<std::string> history_menu_entries` inside `AppState` to feed the Menu component.
2. **Menu Component**:
   ```cpp
   auto history_menu = Menu(&state_.history_menu_entries, &state_.selected_history_idx, history_menu_option);
   ```
3. **Dynamic Flex Frame Decorators**:
   ```cpp
   history_menu->Render() | yframe | flex | border
   ```
   * *Note*: `yframe` allows scrolling, and `flex` ensures the history area takes up all remaining vertical space in the TUI window dynamically at runtime.

---

## Centralized Constants

No line limit constant is needed since layout height is determined dynamically at runtime using `flex`. Any other UI styling constants (e.g., specific selection highlighting colors or borders) will be placed in `src/util/constants.hpp`.

---

## Phased Implementation

### Phase 1 · State & Controller Extensions

Prepare the application state and synchronization points.

#### Step 1 — Implementation
1. **Update `AppState`** (`src/model/app_state.hpp`):
   * Add `std::vector<std::string> history_menu_entries;` (formatted strings for the menu list).
   * Add `int selected_history_idx = 0;` (index of the active selection).
2. **Update `AppController`** (`src/controller/app_controller.hpp` / `.cpp`):
   * Modify functions adding to history (`OnEvaluate`) or clearing history (`OnClearHistory`) to automatically synchronize `state_.history_menu_entries`.
   * Synchronization logic converts `std::vector<std::pair<std::string, std::string>>` history rows to format `"expression = result"`.

#### Step 2 — New Test Cases
* **File**: `tests/controller/test_history_controller.cpp` or `test_app_controller.cpp`
* Verify that performing calculations correctly populates `history_menu_entries` in `AppState`.
* Verify that clearing history clears `history_menu_entries` and resets `selected_history_idx` to `0`.

#### Step 3 — Validation & Human Review
* Build project and run tests.
* **Stop here** for review. Confirm with: *"Phase 1 looks good, proceed to 2"*

---

### Phase 2 · Menu Component Wiring in View

Replace static rendering with the focusable `Menu` in `src/view/app.cpp`.

#### Step 1 — Implementation
1. **Update `src/view/app.cpp`**:
   * Create `history_menu` using `Menu(&state_.history_menu_entries, &state_.selected_history_idx, option)`.
   * Configure `MenuOption` for custom styling (e.g., green text on active selection, bold highlighting).
   * Bind `on_enter` of `MenuOption` to:
     * Retrieve the selected index.
     * Copy the corresponding expression string to the `state_.expression_input` and position the text cursor at the end.
     * Optionally return focus to the expression input field.
   * Add `history_menu` as a child in the layout container (e.g., in a vertical container with `expr_input` and `tab_container`).
2. Verify tab navigation cycles focus correctly: **Top Menu** ↔ **Sub-menus** ↔ **Expression Input** ↔ **History Menu**.

#### Step 2 — Validation & Human Review
* Build project and run tests.
* **Stop here** for review. Confirm with: *"Phase 2 looks good, proceed to 3"*

---

### Phase 3 · Layout, Frame Scrolling & Polish

Constrain the menu inside a scrollable frame boundary.

#### Step 1 — Implementation
1. **Update `src/view/app.cpp`**:
   * Add a layout block that renders `history_menu->Render()` wrapped with the scroll decorators:
     ```cpp
     vbox({
         text("History:") | dim,
         history_menu->Render() | yframe | flex | border
     }) | flex
     ```
   * Style the border and layout to harmonize with the existing user interface.

#### Step 2 — New Test Cases
* Add UI-specific tests to `tests/view/test_app.cpp` simulating keyboard events (`ArrowUp`, `ArrowDown`, `Return`) focused on the history menu component.

#### Step 3 — Validation & Human Review
* Build project and run tests.
* **Stop here** for review. Confirm with: *"Phase 3 looks good, proceed to Final Integration"*

---

## Summary Table

| Phase | Description | Affected files | New tests |
|---|---|---|---|
| 1 | State synchronization | `app_state.hpp`, `app_controller.cpp` | History entry conversion assertions |
| 2 | Component creation & event handling | `app.cpp` | Component hierarchy focus assertions |
| 3 | Frame scrolling layout & styling | `app.cpp`, `test_app.cpp` | Dynamic flex validation tests |

---

## Final Integration

Validate the entire project:
```bash
# Clean rebuild
cmake --build build/ --clean-first

# Run tests
./build/run_tests
```
Verify manually in the terminal that focus transitions and scrolling boundaries behave smoothly.
