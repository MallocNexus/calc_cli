# App UI Specifications

This document outlines the user interface design and component structure of the `calc-cli` application, as implemented in `src/view/app.cpp`.

## UI ASCII Layout

The main interface is a vertical container composed of several stacked components, enclosed within a border.

```text
┌────────────────────────────────────────────────────────────┐
│ File  Edit  Help                                           │ <- Top Menu (Horizontal)
├────────────────────────────────────────────────────────────┤
│ Quit                                                       │ <- Sub-Menu (Tab Container)
├────────────────────────────────────────────────────────────┤
│  > 3 + 4 * (2 - 1)_                                        │ <- Expression Input
├────────────────────────────────────────────────────────────┤
│    = 7                                                     │ <- Result Display
└────────────────────────────────────────────────────────────┘
```

### Modals

Modals are stacked on top of the main view and center themselves on the screen, clearing the content underneath.

**Version Modal (Help -> Version):**
```text
┌───────────────────────────────────────────┐
│              calc-cli  v1.0.0             │
├───────────────────────────────────────────┤
│    Terminal calculator built with FTXUI   │
├───────────────────────────────────────────┤
│                 [ Close ]                 │
└───────────────────────────────────────────┘
```

**History Modal (Edit -> History):**
```text
┌──────────────────────────────────────────────────┐
│   1 + 1  =  2                                    │
│   2 * 3  =  6                                    │
├──────────────────────────────────────────────────┤
│                     [ Close ]                    │
└──────────────────────────────────────────────────┘
```

## Component Breakdown

The `App` class is responsible solely for the view layer. It constructs the FTXUI component tree and delegates all user actions to the `AppController`.

### 1. Top Menu
- **Type:** `Menu` with `MenuOption::HorizontalAnimated()`
- **Entries:** `File`, `Edit`, `Help`
- **Behavior:** Selects which sub-menu is visible in the tab container below it.

### 2. Tab Container (Sub-Menus)
- **Type:** `Container::Tab`
- **Behavior:** Displays the sub-menu corresponding to the selected top menu item.
- **Sub-Menus:**
  - **File:** `Quit` -> Triggers `controller_.OnQuit()`
  - **Edit:**
    - `Clear` -> Triggers `controller_.OnClear()`
    - `History` -> Triggers `controller_.OnOpenHistory()`
  - **Help:**
    - `Version` -> Triggers `controller_.OnOpenVersion()`

### 3. Expression Input
- **Type:** `Input` wrapped in `CatchEvent`
- **Placeholder:** `Enter expression, e.g.  3 + 4 * (2 - 1)`
- **Styling:** White text by default, turns Green when focused. Placeholder is dimmed.
- **Event Handling:**
  - `Return` -> Triggers `controller_.OnEvaluate()`
  - `Escape` -> Triggers `controller_.OnClear()`

### 4. Result Display
- **Type:** `text` rendered within the `Renderer` block.
- **Content:** Driven by `state_.result_display`.
- **Styling:**
  - Empty: Dimmed.
  - Success (`state_.error_state == false`): Green and bold.
  - Error (`state_.error_state == true`): Red.

### 5. Main Layout Renderer
- **Structure:** `vbox` containing the top menu, sub-menu, input, and result display, separated by `separator()` lines, and enclosed in a `border`.

### 6. Modals
- **Component Stack:** Uses `Modal` components to layer popups over the main renderer.
- **Version Modal:** A static text popup with a "Close" button triggering `controller_.OnCloseVersion()`.
- **History Modal:** Displays past evaluations retrieved from `controller_.GetHistory()`. If empty, shows "No history yet.". Includes a "Close" button triggering `controller_.OnCloseHistory()`.
