#include "controller/app_controller.hpp"
#include "model/calculator.hpp"
#include "util/formatting.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------
// AppController implementation
// All event-handling logic lives here — no FTXUI in this file.
// ---------------------------------------------------------------------------

AppController::AppController(AppState& state, Calculator& calc,
                             std::function<void()> on_quit)
    : state_(state), calc_(calc), on_quit_(on_quit) {}

void AppController::OnEvaluate() {
    std::string formatted = util::FormatExpression(state_.expression_input);
    state_.expression_input = formatted;

    EvaluationResult res = calc_.Evaluate(formatted);
    if (res.ok) {
        state_.result_display = "= " + util::FormatDouble(res.value);
        state_.error_state = false;
    } else {
        state_.result_display = "Error: " + res.error;
        state_.error_state = true;
    }
}

void AppController::OnClear() {
    state_.expression_input.clear();
    state_.cursor_position = 0;
    state_.result_display.clear();
    state_.error_state = false;
}

void AppController::OnQuit() { on_quit_(); }

void AppController::OnClearHistory() { calc_.ClearHistory(); }

void AppController::OnOpenVersion() { state_.show_version_modal = true; }

void AppController::OnCloseVersion() { state_.show_version_modal = false; }

const std::vector<std::pair<std::string, std::string>>&
AppController::GetHistory() const {
    return calc_.GetHistory();
}
