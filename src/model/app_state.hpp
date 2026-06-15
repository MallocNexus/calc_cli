#pragma once

#include <string>

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

    HELP_VERSION = 0,
};
