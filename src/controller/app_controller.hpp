#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "model/app_state.hpp"

// Forward declaration — avoids pulling in calculator.hpp into every consumer
// of this header. The full definition is only needed in app_controller.cpp.
class Calculator;

// ---------------------------------------------------------------------------
// AppController
// Owns all business logic that responds to user events.
// Has ZERO FTXUI dependency — only reads/writes AppState and calls Calculator.
// This makes it independently unit-testable without a terminal.
// ---------------------------------------------------------------------------
class AppController {
  public:
    AppController(AppState& state, Calculator& calc,
                  std::function<void()> on_quit);

    // Called by the view in response to user input events:
    void OnEvaluate();     // Return pressed — evaluate expression_input
    void OnClear();         // Escape pressed or Edit->Clear Input
    void OnClearHistory();  // Edit->Clear History
    void OnQuit();          // File->Quit selected

    void OnOpenVersion();   // Help->Version selected
    void OnCloseVersion();  // Version modal Close button

    // Read-only access to evaluation history for the view to render.
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    AppState& state_;
    Calculator& calc_;
    std::function<void()> on_quit_;

};
