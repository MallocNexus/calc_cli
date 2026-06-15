#pragma once

#include <functional>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>

// Forward declaration — app.hpp does not include calculator.hpp directly so
// that consumers only need to include what they actually use.
class Calculator;

// ---------------------------------------------------------------------------
// AppState
// Flat, plain-data struct that holds all mutable UI state.
// Both the App and tests access this directly — no getters required.
// ---------------------------------------------------------------------------
struct AppState {
    std::string expression_input;
    int cursor_position = 0;

    std::string result_display;  // "= 7" on success, "Error: ..." on failure
    bool error_state = false;    // true when result_display is an error

    // Modal visibility — stored here so tests can assert without App internals.
    bool show_version_modal = false;
    bool show_history_modal = false;
};

// ---------------------------------------------------------------------------
// MenuIndex
// Named indices for each sub-menu to avoid hardcoded magic numbers.
// Update this enum whenever menu items are added or reordered.
// ---------------------------------------------------------------------------
enum MenuIndex {
    FILE_QUIT = 0,

    EDIT_CLEAR = 0,
    EDIT_HISTORY = 1,

    HELP_VERSION = 0,
};

// ---------------------------------------------------------------------------
// App
// Owns the entire FTXUI component tree.  Accepts dependencies by reference;
// does not own the objects pointed to.
// ---------------------------------------------------------------------------
class App {
  public:
    App(AppState& state, Calculator& calc, std::function<void()> on_quit);

    // Returns the root component.  Pass this to ScreenInteractive::Loop or
    // call OnEvent() on it directly in tests.
    ftxui::Component GetComponent();

  private:
    AppState& state_;
    Calculator& calc_;
    std::function<void()> on_quit_;

    // Top-level horizontal menu entries.
    std::vector<std::string> top_menu_entries_ = {"File", "Edit", "Help"};
    int top_menu_selected_ = 0;

    // Sub-menu entries for each top-level item.
    std::vector<std::string> file_entries_ = {"Quit"};
    int file_selected_ = 0;

    std::vector<std::string> edit_entries_ = {"Clear", "History"};
    int edit_selected_ = 0;

    std::vector<std::string> help_entries_ = {"Version"};
    int help_selected_ = 0;

    ftxui::Component component_;
};
