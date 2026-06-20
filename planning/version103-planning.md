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

