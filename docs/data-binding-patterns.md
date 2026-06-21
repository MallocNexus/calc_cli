# FTXUI Data Binding Patterns in calc-cli

This document outlines the standard data-binding patterns used in the `calc-cli` Terminal User Interface (TUI) via the FTXUI library.

---

## 1. Overview of FTXUI Data Binding

Unlike traditional MVC frameworks that require manual event-based UI updates, FTXUI uses a **reactive data-binding model** supported by its layout rendering pipeline. 

Instead of copying variables into components and manually redrawing them when state changes, components are initialized with **pointers** (references) to the application state. When the screen interactive loop runs a frame rendering, it dereferences these pointers to fetch the most up-to-date values.

---

## 2. Under the Hood: `ftxui::Ref<T>` and `ftxui::ConstRef<T>`

FTXUI represents bound properties using the [ftxui::Ref<T>](../build/_deps/ftxui-src/include/ftxui/util/ref.hpp#L46) (mutable) and [ftxui::ConstRef<T>](../build/_deps/ftxui-src/include/ftxui/util/ref.hpp#L17) (immutable) adapters.

These classes wrap a `std::variant<T, T*>` (or `const T*`). When constructing an option struct or component options:
* **Value Binding (Owned)**: If you pass a literal or value (e.g., `focused_entry = 0`), the variant stores the value locally.
* **Pointer Binding (Reference)**: If you pass a pointer (e.g., `focused_entry = &state.focused_history_idx`), the variant stores the pointer.

On every draw, FTXUI calls the `operator*` or `operator()` overload on the `Ref` wrapper, which dynamically queries the memory location.

---

## 3. Data Binding Usage in `calc-cli`

All mutable UI states are centralized in the flat [AppState](../src/model/app_state.hpp#L12) structure in the model layer. The TUI component tree in [app.cpp](../src/view/app.cpp) binds directly to its fields.

### Pattern A: Text Input Fields
In [app.cpp](../src/view/app.cpp), the expression field is bound to the state's input string and cursor index:
```cpp
InputOption input_option = InputOption::Default();
input_option.content = &state_.expression_input;     // Bind text content
input_option.cursor_position = &state_.cursor_position; // Bind cursor position
auto expr_input_base = Input(input_option);
```

### Pattern B: Menu Indices & Selection
The active child index of a Tab container, menus, and selection indices are bound directly:
* **Top-Level Tabs**:
  ```cpp
  auto tab_container = Container::Tab({file_menu, edit_menu, exchange_menu, help_menu}, &top_menu_selected_);
  ```
* **History Menu**:
  ```cpp
  auto history_menu = Menu(&state_.history_menu_entries, &state_.selected_history_idx, history_menu_option);
  ```

### Pattern C: Menu Internal Focus Tracking (Selection Highlight Sync)
The history menu options sync both selection state and internal visual/hover focus state:
```cpp
history_menu_option.focused_entry = &state_.focused_history_idx; // Bind focus tracking
```
By resetting both `selected_history_idx` and `focused_history_idx` to `0` in [app_controller.cpp](../src/controller/app_controller.cpp) on recall/evaluation, we guarantee that the visual highlighted item (active state) and the FTXUI internal hovered item (focused state) remain perfectly in sync.

### Pattern D: Modal Visibility Flags
FTXUI modals are controlled via visibility boolean flags:
```cpp
auto component_temp = Modal(main_renderer, version_modal, &state_.show_version_modal);
component_ = Modal(component_temp, custom_modal, &state_.show_custom_modal);
```
Changing these booleans in `AppState` triggers immediate visibility transitions.

---

## 4. Key Rules & Best Practices

1. **Modify State via Controller Only**:
   The TUI layout/renderer should remain pure and stateless. Any change in state (including index resets) must be written inside [app_controller.cpp](../src/controller/app_controller.cpp).
2. **Keep the State Flat**:
   All reactive UI variables must reside flatly in [AppState](../src/model/app_state.hpp#L12) to make them easily queryable by both controllers and integration tests.
3. **Synchronize Selection & Focus**:
   When programmatically modifying a menu's selection index (e.g. resetting `selected_history_idx` to `0`), always bind and update `focused_entry` to the same index to prevent duplicate visual highlights.
