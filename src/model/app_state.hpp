#pragma once

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// AppState
// Flat, plain-data struct holding all mutable UI state.
// Lives in the model layer so it can be included by all three layers
// without pulling in FTXUI or controller headers.
// ---------------------------------------------------------------------------
struct AppState {
    std::string expression_input;
    int cursor_position = 0;

    std::string result_display;  // "= 7" on success, "Error: ..." on failure
    bool error_state = false;    // true when result_display is an error

    // Modal visibility — stored here so tests can assert them directly.
    bool show_version_modal = false;

    // Custom Exchange Modal State
    bool show_custom_modal = false;
    std::string custom_source_input = "";
    std::string custom_target_input = "";

    // Calculation History Menu/Scrolling State
    std::vector<std::string> history_menu_entries;
    int selected_history_idx = 0;
    int focused_history_idx = 0;
};

// ---------------------------------------------------------------------------
// MenuIndex
// Named indices for each sub-menu item.
// Update whenever menu items are added or reordered — never use raw integers.
// ---------------------------------------------------------------------------
enum MenuIndex {
    FILE_QUIT = 0,

    EDIT_CLEAR_INPUT = 0,
    EDIT_CLEAR_HISTORY = 1,

    EXCHANGE_AUD_USD = 0,
    EXCHANGE_CUSTOM = 1,

    HELP_VERSION = 0,
};
