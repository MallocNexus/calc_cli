#pragma once

#include <string>
#include <vector>

#include <ftxui/component/component.hpp>

#include "model/app_state.hpp"

// Forward declaration — avoids pulling FTXUI into controller or model headers.
class AppController;

// ---------------------------------------------------------------------------
// App (View)
// Builds the FTXUI component tree and wires user events to AppController.
// Contains NO business logic — every action delegates to the controller.
// ---------------------------------------------------------------------------
class App {
  public:
    App(AppState& state, AppController& controller);

    // Returns the root component. Pass to ScreenInteractive::Loop or call
    // OnEvent() directly in integration tests.
    ftxui::Component GetComponent();

  private:
    AppState& state_;
    AppController& controller_;

    // Top-level horizontal menu entries.
    std::vector<std::string> top_menu_entries_ = {"File", "Edit", "Exchange", "Help"};
    int top_menu_selected_ = 0;

    // Sub-menu entries for each top-level item.
    std::vector<std::string> file_entries_ = {"Quit"};
    int file_selected_ = 0;

    std::vector<std::string> edit_entries_ = {"Clear Input", "Clear History"};
    int edit_selected_ = 0;

    std::vector<std::string> exchange_entries_ = {"AUD -> USD", "Custom"};
    int exchange_selected_ = 0;

    std::vector<std::string> help_entries_ = {"Version"};
    int help_selected_ = 0;

    ftxui::Component component_;
};
