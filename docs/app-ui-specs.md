# User Interface Specifications (v1.0.3)

This document defines the layout, visual styling, components, and interaction patterns of the `calc-cli` Terminal User Interface (TUI).

## 1. General Layout

The interface is enclosed in a single outer border and uses horizontal separator lines to divide the screen into five main sections.

```text
┌────────────────────────────────────────────────────────────┐
│  File  Edit  Exchange  Help                                │ <- Top Menu Bar
├────────────────────────────────────────────────────────────┤
│  Clear Input  Clear History                                │ <- Sub-Menu Options
├────────────────────────────────────────────────────────────┤
│ > 3 + 4 * (2 - 1)_                                         │ <- Expression Input
├────────────────────────────────────────────────────────────┤
│   = 7                                                      │ <- Result Display (Green on success, Red on error)
├────────────────────────────────────────────────────────────┤
│ History:                                                   │ <- History Panel Header
├────────────────────────────────────────────────────────────┤
│   3 + 4 * (2 - 1) = 7                                      │ <- History List Items
│   10 + 20 = 30                                             │
│   50 exchange(AUD, USD) = 32.50                            │
└────────────────────────────────────────────────────────────┘
```

## 2. Component Design & Behaviors

### A. Top Menu Bar & Sub-Menus
* **Top Menu**: Horizontal menu with animated transitions using `ftxui::MenuOption::HorizontalAnimated()`.
* **Tab Container**: A `ftxui::Container::Tab` displays the horizontal sub-menu list matching the selected top category:
  - **File**:
    - `Quit`: Closes the application via `controller_.OnQuit()`.
  - **Edit**:
    - `Clear Input`: Clears the input field and result display via `controller_.OnClear()`.
    - `Clear History`: Erases all database and cached calculations via `controller_.OnClearHistory()`.
  - **Exchange**:
    - `AUD->USD`: Appends `exchange(AUD, USD)` to the input and refocuses it.
    - `Custom`: Opens the Custom Exchange Rate modal popup.
  - **Help**:
    - `Version`: Opens the Version modal popup.

### B. Expression Input Field
* **Type**: `ftxui::Input` component restricted to a single line.
* **Placeholder**: `Enter expression, e.g.  3 + 4 * (2 - 1)`.
* **Styling**: Dimmed when showing the placeholder. Turns **green** when focused, **white** when unfocused.
* **Event Interceptions**:
  - `Return`: Triggers calculation via `controller_.OnEvaluate()`.
  - `Escape`: Clears the input field via `controller_.OnClear()`.

### C. Result Display
* **Type**: A text renderer that displays the result of the evaluation.
* **Styling**:
  - Empty: Displays nothing.
  - Success (valid expression): Displayed as `= <result>` in **bold green**.
  - Error (invalid expression/division by zero): Displayed as `Error: <reason>` in **red**.

### D. Inline History Panel
* **Type**: A vertical list container (`ftxui::MenuOption::Vertical()`) at the bottom of the screen.
* **Content**: Renders the history list from `state_.history_menu_entries`. If empty, displays `No history yet` (dimmed).
* **Styling**: Wrapped with a vertical scrollbar indicator (`ftxui::vscroll_indicator | ftxui::frame`). Highlighted items turn **bold green** on focus.
* **Interactions**:
  - Pressing `Return` on a history item copies the expression back into the input field and resets the history index highlights to the top.

### E. Dialog Modals

Modals are stacked on top of the main layout, center themselves, and clear the screen underneath.

#### 1. Version Modal (Help -> Version)
```text
┌───────────────────────────────────────────┐
│              calc-cli  v1.0.3             │
├───────────────────────────────────────────┤
│    Terminal calculator built with FTXUI   │
├───────────────────────────────────────────┤
│                 [ Close ]                 │
└───────────────────────────────────────────┘
```
* **Trigger**: Activated when `state_.show_version_modal` is true.
* **Components**: Displays the application title and current version. Features a centered "Close" button to close the modal.

#### 2. Custom Exchange Modal (Exchange -> Custom)
```text
┌──────────────────────────────────────────┐
│             Custom Exchange              │
├──────────────────────────────────────────┤
│ Source Currency:  [ AUD           ]      │
│ Target Currency:  [ EUR           ]      │
├──────────────────────────────────────────┤
│            [ OK ]     [ Cancel ]         │
└──────────────────────────────────────────┘
```
* **Trigger**: Activated when `state_.show_custom_modal` is true.
* **Components**: Two single-line text input fields for Source and Target currency codes.
* **Submission**: Pressing `OK` or `Return` appends `exchange(SOURCE, TARGET)` to the main input box and closes the modal. Pressing `Cancel` or `Escape` closes the modal without appending.
