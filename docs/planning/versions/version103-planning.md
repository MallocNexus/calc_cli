# Plan: Fix History Menu Selection Index Reset (v1.0.3)

## Overview & Bug Analysis

When a calculation is selected and recalled from the history menu, the selected expression is copied to the input box and focus shifts to the input field. 

Currently:
1. `selected_history_idx` remains at the index of the recalled item.
2. If the subsequent evaluation succeeds, `OnEvaluate()` calls `SyncHistoryMenuEntries()`, which resets `selected_history_idx` to `0` (correct).
3. However, if the subsequent evaluation fails (e.g. offline network timeout during exchange rate resolution, or if the user edits the recalled expression to be syntactically invalid), `SyncHistoryMenuEntries()` is **not** called. As a result, `selected_history_idx` remains stuck at the recalled index.
4. When the user navigates back down to the history menu, the focus/selection jumps back to the previously selected item rather than starting from the top (`0`).

### Root Cause
The history menu index reset is tied to a **successful evaluation state update** rather than the **user interaction lifecycle** (recalling/using the item).

---

## Proposed Solution

Reset `selected_history_idx` to `0` **immediately** inside `OnUseSelectedHistory()` in `src/controller/app_controller.cpp` when the user recalls a calculation.

This decouples the menu selection index from the outcome of the next evaluation, ensuring that:
* Once a history item is consumed/recalled, the history menu selection immediately reverts to the top of the list.
* If the subsequent evaluation fails, succeeds, or is modified, the selection index remains at `0` for the next interaction.

---

## Phased Implementation

### Phase 1 · Controller Modification
Modify `AppController::OnUseSelectedHistory()` to reset the index to `0` when using a history item.

* **File**: `src/controller/app_controller.cpp`
* **Changes**:
  ```cpp
  void AppController::OnUseSelectedHistory() {
      const auto& hist = history_ctrl_.GetHistory();
      if (state_.selected_history_idx >= 0 &&
          state_.selected_history_idx < static_cast<int>(hist.size())) {
          const auto& selected_pair = hist[state_.selected_history_idx];
          state_.expression_input = selected_pair.first;
          state_.cursor_position = static_cast<int>(selected_pair.first.size());
          state_.selected_history_idx = 0; // Reset index immediately on use
      }
  }
  ```

---

### Phase 2 · Integration Tests
Add test cases to verify index reset behavior on both successful and failed evaluations.

* **File**: `tests/view/test_app.cpp` or `tests/controller/test_app_controller.cpp`
* **Test Scenarios**:
  1. Recall an item from history, trigger an evaluation that succeeds -> verify selection index remains `0` when focused.
  2. Recall an item from history, trigger an evaluation that fails (invalid syntax or network timeout) -> verify selection index still resets to `0` when focused.

---

## Validation

1. Run the clean rebuild and run tests:
   ```bash
   cmake --build build/ --clean-first
   ./build/run_tests
   ```
2. Verify all test cases pass.

---

## Section 2: Duplicate Green Highlights (Focus / Hover State Sync)

### Overview & Bug Analysis
When the user recalls an item from the history menu, the input is populated and `selected_history_idx` is reset to `0`. However, if they immediately navigate back (e.g. by pressing Tab) to focus the history menu, two items are highlighted in green: index `0` and the index of the previously selected item.

### Root Cause
FTXUI's `Menu` component tracks a `focused_entry` index internally. When we reset `selected_history_idx` to `0` programmatically while the menu is unfocused, the menu's internal `focused_entry` remains stuck at the index of the previously selected item.
When the menu receives focus again:
* Index `0` is active (`s.active == true` because `selected_history_idx` is `0`).
* The previously selected index is focused (`s.focused == true` because the menu's internal `focused_entry` is still at the old index).
This causes both items to render with green highlight styles.

### Proposed Solution
Control the menu's internal focus state externally by linking it to `AppState`:
1. Add `int focused_history_idx = 0;` to `AppState` in `src/model/app_state.hpp`.
2. Bind `history_menu_option.focused_entry = &state_.focused_history_idx;` in `src/view/app.cpp`.
3. Reset `state_.focused_history_idx = 0;` whenever we reset `state_.selected_history_idx = 0;` in `src/controller/app_controller.cpp`.

---

## Section 2 Implementation Plan

### Phase 3 · Model and Controller Extensions
1. Add `int focused_history_idx = 0;` to `AppState` ([app_state.hpp](file:///Users/mattswart/Source/CPP/calc-cli/src/model/app_state.hpp)).
2. Update `AppController::OnUseSelectedHistory()` and `AppController::SyncHistoryMenuEntries()` in `src/controller/app_controller.cpp` to reset `focused_history_idx` to `0`.

### Phase 4 · View Update
1. Update [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp) to bind:
   ```cpp
   history_menu_option.focused_entry = &state_.focused_history_idx;
   ```

### Phase 5 · Verification & Tests
1. Add test assertions in `tests/view/test_app.cpp` to verify that `focused_history_idx` is reset to `0` immediately upon recall.
2. Build and run tests to verify no regressions.

---

## Section 3: Append Exchange Shorthand to End of Input

### Overview & Bug Analysis
When the user has entered an expression (e.g. `600`) and navigates to the Exchange tab to select `AUD->USD`, the shorthand `"exchange(AUD, USD)"` is currently inserted at `state_.cursor_position`.
However, because focus shifted to the menus, the input cursor index may be reset or set to `0`. Consequently, `"exchange(AUD, USD)"` is prepended to the beginning (e.g. `"exchange(AUD, USD)600"`) instead of being appended to the end.

Since `exchange(...)` is an operator that applies to the preceding expression, it should **always** be appended to the end of the input box.

### Proposed Solution
Modify both [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp) and [custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.cpp) to always append the exchange shorthand to the end of the input:
1. Helper check:
   - If `state_.expression_input` is not empty and does not end with a space ` `, append a space before the shorthand (e.g. `" exchange(AUD, USD)"`).
   - Otherwise, append the shorthand directly.
2. Update `state_.cursor_position` to the end of the newly modified input string (`state_.expression_input.size()`), so the user can immediately continue typing at the end of the line.

---

## Section 3 Implementation Plan

### Phase 6 · View Updates (Default & Custom Exchange)
1. In [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp), modify the `on_enter` of `exchange_option`:
   * Determine the string to append (`"exchange(AUD, USD)"`).
   * Check if `state_.expression_input` is not empty and doesn't end with space; if so, prepend a space.
   * Append to the end of `state_.expression_input`.
   * Set `state_.cursor_position = state_.expression_input.size()`.
2. In [custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.cpp), modify both the `OK` button callback and the `Return` key handler to use the same appending logic.

### Phase 7 · Verification & Tests
1. Update existing test case `App — Exchange -> AUD -> USD inserts shorthand` in [test_app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_app.cpp) to verify it appends to the end regardless of initial cursor position.
2. Add a new integration test case (`App — Exchange -> AUD -> USD appends to end after typing`) that:
   * Simulates typing `600` in the input field.
   * Navigates to the Exchange tab menu.
   * Simulates selecting `AUD->USD` and pressing Enter.
   * Verifies the input contains `"600 exchange(AUD, USD)"` and the cursor is at the end.

---

## Section 4: Focus Refocus back to Expression Input after Exchange Selection

### Overview & Bug Analysis
Currently, when the user selects `AUD->USD` from the Exchange sub-menu, the shorthand is appended to the input field, but focus remains on the menu item. The user has to manually press Down to return focus to the expression input field.
Similarly, when submitting the Custom Exchange modal, focus currently returns to the `Custom` menu item inside the Exchange tab rather than the input field.

To streamline the user experience, selecting or submitting an exchange action should automatically return focus to the expression input field.

### Proposed Solution
1. In [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp), update `exchange_option.on_enter` to capture `expr_input_base` and call `expr_input_base->TakeFocus();` after appending the shorthand.
2. In [custom_exchange.hpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.hpp) and [custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.cpp), extend the constructor to accept a focus/completion callback:
   `CustomExchange(AppState& state, std::function<void()> on_submit_success);`
3. Call `on_submit_success()` inside the `OK` button callback and `Return` event handler when inputs are valid and submitted successfully.
4. Pass `[expr_input_base] { expr_input_base->TakeFocus(); }` when instantiating `CustomExchange` in [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp).

---

## Section 4 Implementation Plan

### Phase 8 · View Focus Refactoring
1. Update `CustomExchange` constructor definition in [custom_exchange.hpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.hpp) and [custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/custom_exchange.cpp).
2. Update the `OK` button callback and `Return` key press handler inside `CustomExchange` to invoke the callback on successful submission.
3. Update [app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/src/view/app.cpp):
   * Modify `exchange_option.on_enter` to capture `expr_input_base` and call `expr_input_base->TakeFocus()`.
   * Pass the callback when creating `custom_exchange_`.

### Phase 9 · Verification & Tests
1. Add test assertions in [test_app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_app.cpp) to verify that the expression input component gains focus after inserting shorthand.
2. Update `CustomExchange View — interaction` tests in [test_custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_custom_exchange.cpp) to verify that the submit success callback is invoked.



