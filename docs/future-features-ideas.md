# Future Feature Ideas

This document logs proposed feature enhancements for the `calc-cli` terminal calculator.

---

## 1. History Sorting Options

### Overview
Currently, the calculation history menu displays items in a fixed order (newest calculations at the bottom). We should introduce a sorting toggle that allows users to switch between:
1. **Newest First to Oldest** (Reverse chronological order)
2. **Oldest First to Newest** (Chronological order)

### Proposed Implementation Details

#### A. Model Layer (`HistoryRepository`)
Extend the SQL queries inside `src/model/history_repository.cpp` to accept a sorting parameter:
* **Chronological (Oldest First)**:
  `SELECT id, expression, result FROM history ORDER BY id ASC;`
* **Reverse Chronological (Newest First)**:
  `SELECT id, expression, result FROM history ORDER BY id DESC;`

#### B. State Layer (`AppState`)
Add a state field in `src/model/app_state.hpp` to track the current sorting preference:
```cpp
enum class HistorySortOrder {
    NEWEST_FIRST,
    OLDEST_FIRST
};

struct AppState {
    // ...
    HistorySortOrder history_sort_order = HistorySortOrder::NEWEST_FIRST;
};
```

#### C. Controller Layer (`HistoryController` & `AppController`)
* Add a `SetSortOrder(HistorySortOrder order)` method to `HistoryController` to configure the repository query.
* In `AppController`, implement `OnToggleHistorySort()` that toggles `state_.history_sort_order`, re-reads the history from the database in the new order, and calls `SyncHistoryMenuEntries()`.

#### D. View Layer (`App` Component)
* Add a new option to the top menu (e.g., `Edit -> Toggle History Sort`) or assign a keyboard shortcut (e.g., `S` or `Ctrl+S` when focused on the history menu) to trigger the sort toggle.
* Update the history header title to dynamically show the current order (e.g., `Calculation History (Newest First)`).

---

## 2. Currency Exchange Shorthand (`ex`)

### Overview
To simplify manual keyboard entry in CLI and input modes, we should support `ex` as a shorthand alias for the `exchange` operator. Both `ex(base, quote)` and `exchange(base, quote)` will resolve identically.

### Proposed Implementation Details

#### A. Parser Service (`Parser` class)
Modify token scanning and parsing helper functions in `src/service/parser.cpp`:
* In `Parser::ParseExpr()` and `Parser::ParsePrimary()`, update keyword scanning to look for both `"exchange"` and `"ex"` prefixes:
  ```cpp
  // Inside Parser::ParsePrimary() or similar token match
  if (MatchKeyword("exchange") || MatchKeyword("ex")) {
      Advance(); // consume keyword
      // ... parse '(' base ',' quote ')'
  }
  ```

#### B. Validation & Tests
Add tests inside `tests/model/test_parser.cpp` to verify parsing and evaluation of `ex` expressions:
* Basic parsing test: `100 ex(AUD, USD)` should match output of `100 exchange(AUD, USD)`.
* Precedence tests: Verify `2 + 10 ex(USD, AUD)` parses with correct operator precedence.

---

## 3. Persistent Custom Exchange UI Shortcuts

### Overview
Currently, the Exchange sub-menu in the TUI lists a single hardcoded shortcut (`AUD->USD`) and a `Custom` option to trigger the modal. We should allow users to add their own custom currency pairs (e.g. `AUD->EUR`, `USD->GBP`) directly to the TUI sub-menu as persistent, selectable shortcuts.

### Proposed Implementation Details

#### A. Model Layer (`ExchangeRate` Database)
Add a new SQLite table to store user-defined UI shortcuts:
```sql
CREATE TABLE IF NOT EXISTS exchange_shortcuts (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    base TEXT NOT NULL,
    quote TEXT NOT NULL,
    UNIQUE(base, quote)
);
```
Implement methods in `src/model/exchange_rate.cpp` to:
* `GetShortcuts()`: Fetch all user shortcuts.
* `AddShortcut(base, quote)`: Insert a new shortcut.
* `DeleteShortcut(id)`: Remove a shortcut.

#### B. State Layer (`AppState`)
Change `AppState`'s static menu list to a dynamic vector populated at application startup:
```cpp
struct AppState {
    // Loaded dynamically from DB, starts with ["AUD->USD", "Custom"]
    std::vector<std::string> exchange_menu_entries;
    int exchange_selected_idx = 0;
};
```

#### C. Controller Layer (`ExchangeRateController` & `AppController`)
* Update `ExchangeRateController` to read custom shortcuts from the database and sync them to `AppState::exchange_menu_entries` (inserting them right before the `Custom` modal entry).
* In the `CustomExchange` modal submission handler, add a checkbox option: `"Save as shortcut"`. If checked, write the new pair to the database and re-sync the menu.

#### D. View Layer (`App` Component)
* Dynamically generate the `Exchange` sub-menu using `state_.exchange_menu_entries`.
* If a custom shortcut is pressed, determine the corresponding currency pair and append its `exchange(base, quote)` shorthand to the input box automatically.

---

## 4. Quick History Navigation (Left/Right Arrows)

### Overview
In the calculation history panel, navigating a large history list item-by-item with Up/Down arrows can be slow. We should introduce quick navigation keys:
* **Left Arrow**: Jumps immediately to the top of the history list (index `0`).
* **Right Arrow**: Jumps immediately to the bottom of the history list (the last calculated entry).

### Proposed Implementation Details

#### A. View Layer (`App` Component)
Within the event handler for the history menu component in `src/view/app.cpp`, intercept Left and Right arrow events:
```cpp
history_menu = CatchEvent(history_menu, [&](Event event) {
    if (event == Event::ArrowLeft) {
        state_.selected_history_idx = 0;
        state_.focused_history_idx = 0; // Sync focus hover highlights
        return true; // Event handled
    }
    if (event == Event::ArrowRight) {
        int last_idx = static_cast<int>(state_.history_menu_entries.size()) - 1;
        state_.selected_history_idx = last_idx;
        state_.focused_history_idx = last_idx; // Sync focus hover highlights
        return true; // Event handled
    }
    return false;
});
```

#### B. Verification & Tests
Add test cases in `tests/view/test_app.cpp` to verify navigation:
* Select the history menu component and simulate an `ArrowLeft` event -> assert `selected_history_idx` and `focused_history_idx` update to `0`.
* Simulate an `ArrowRight` event -> assert both indices update to the final calculation index (`history_menu_entries.size() - 1`).

---

## 5. Context-Sensitive F1 Help Modal

### Overview
To guide users through available TUI shortcuts and features, pressing the **F1** key should display a modal popup showing context-sensitive help. The dialog content will adapt dynamically to display shortcut keys and helper instructions specific to whichever component currently holds keyboard focus.

### Proposed Implementation Details

#### A. State Layer (`AppState`)
Add fields in `src/model/app_state.hpp` to control help visibility and active focus context:
```cpp
enum class HelpContext {
    INPUT_FIELD,
    HISTORY_LIST,
    EXCHANGE_MENU,
    GLOBAL
};

struct AppState {
    // ...
    bool show_help_modal = false;
    HelpContext active_help_context = HelpContext::GLOBAL;
};
```

#### B. Controller Layer (`AppController`)
Implement methods to open and close the help dialog:
* `OnShowHelp(HelpContext context)`: Set the active context and toggle visibility.
* `OnCloseHelp()`: Set `show_help_modal = false`.

#### C. View Layer (`App` Component)
* Wrap the top-level container with a global event hook to intercept `ftxui::Event::F1`.
* Determine focus dynamically:
  * If `expr_input_base` is focused: set context to `INPUT_FIELD`.
  * If `history_menu` is focused: set context to `HISTORY_LIST`.
  * Otherwise, default to `GLOBAL` or `EXCHANGE_MENU`.
* Render the modal component using a simple scrollable read-only paragraph card containing help text (e.g. `Return` to evaluate in input, `Left/Right` to skip to boundaries in history, etc.) and a close button.

#### D. Verification & Tests
Add integration test cases in `tests/view/test_app.cpp`:
* Select the expression input field, simulate an `F1` keystroke -> assert `show_help_modal` is true and `active_help_context` is `HelpContext::INPUT_FIELD`.
* Press `Return` or select the close button -> assert `show_help_modal` is false.




