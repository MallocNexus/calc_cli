# Inline History View Plan
## Moving History from Modal to Main UI

---

## 1. Goal

Integrate the evaluation history directly into the main application view below the "Result Display", making it always visible to the user without needing to navigate menus and open a modal.

---

## 2. UI ASCII Layout Update

The new interface will add an expandable or scrolling section at the bottom to show previous evaluations.

```text
┌────────────────────────────────────────────────────────────┐
│ File  Edit  Help                                           │ <- Top Menu
├────────────────────────────────────────────────────────────┤
│ Clear                                                      │ <- Sub-Menu
├────────────────────────────────────────────────────────────┤
│  > 3 + 4 * (2 - 1)_                                        │ <- Expression Input
├────────────────────────────────────────────────────────────┤
│    = 7                                                     │ <- Result Display
├────────────────────────────────────────────────────────────┤
│ History:                                                   │ <- New History Section
│   1 + 1  =  2                                              │
│   2 * 3  =  6                                              │
└────────────────────────────────────────────────────────────┘
```

---

## 3. Design Decisions

### 3.1 Scrolling the History
As the history grows, it could push the main interface elements off the screen. To prevent this, the history section should be wrapped in an FTXUI `vscroll_indicator` or a `yframe` (scrollable vertical frame) combined with `flex` to ensure it only takes up available space and allows scrolling if it overflows the terminal height.

### 3.2 What happens to the History Modal?
Since the history is now continuously visible on the main screen, the "Edit -> History" menu option and its corresponding modal are redundant. 
**Decision:** Remove the History modal and the "Edit -> History" menu item. 
- "Edit" will now only contain "Clear".
- `OnOpenHistory()` and `OnCloseHistory()` will be removed from `AppController`.
- `show_history_modal` will be removed from `AppState`.

### 3.3 History Clearing
The "Edit -> Clear" menu item currently clears the current input and result display. The user may also want to clear the entire history. We have two choices:
1. Add an "Edit -> Clear History" menu item.
2. Have "Edit -> Clear" (or pressing Escape) clear the history as well.

**Decision:** Add a dedicated "Edit -> Clear History" menu item. The `Calculator` model already has a `ClearHistory()` method. We will add `controller_.OnClearHistory()` to trigger it.

---

## 4. Required Code Changes

### 4.1 `src/model/app_state.hpp`
- Remove `show_history_modal`.
- Add `EDIT_CLEAR_HISTORY` to the `MenuIndex` enum.

### 4.2 `src/controller/app_controller.hpp` & `.cpp`
- Remove `OnOpenHistory()` and `OnCloseHistory()`.
- Add `OnClearHistory()` which will call `calc_.ClearHistory()`.

### 4.3 `src/view/app.cpp`
- **Menu Update:** Change `edit_entries_` to `{"Clear Input", "Clear History"}`.
- **Menu Action:** Update `edit_option.on_enter` to map the new options to `controller_.OnClear()` and `controller_.OnClearHistory()`.
- **Renderer Update:** Inside `main_renderer`, fetch `controller_.GetHistory()`. Build a `vbox` containing the history rows. Wrap this box in `yframe | flex` so it can scroll if needed. Append this block below the `Result Display` block inside the main `vbox`.
- **Modal Removal:** Remove `history_close`, `history_modal`, and the stacking logic that applied it to the component tree. The main component tree will just be `Modal(main_renderer, version_modal)`.

---

## 5. Implementation Checklist

- [ ] Update `src/model/app_state.hpp`: Remove `show_history_modal` flag, update `MenuIndex` for `EDIT_CLEAR_HISTORY`.
- [ ] Update `src/controller/app_controller.hpp` & `.cpp`: Remove history modal methods, add `OnClearHistory()`.
- [ ] Update `src/view/app.hpp`: Update `edit_entries_`.
- [ ] Update `src/view/app.cpp`:
  - Wire up `Edit -> Clear History` menu action.
  - Remove History modal construction.
  - Render history lines directly inside the `main_renderer` beneath the result display using a `yframe` for scrolling.
- [ ] Update `tests/test_main.cpp`:
  - Update integration tests to reflect menu changes (e.g., removing the history modal test, adding a clear history test).
- [ ] Build & run tests: `cmake --build build && ./build/run_tests`.
