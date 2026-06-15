#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "model/calculator.hpp"
#include "util/formatting.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------
// AppController implementation
// All event-handling logic lives here — no FTXUI in this file.
// ---------------------------------------------------------------------------

AppController::AppController(AppState& state, Calculator& calc,
                             HistoryController& history_ctrl,
                             std::function<void()> on_quit)
    : state_(state), calc_(calc), history_ctrl_(history_ctrl), on_quit_(on_quit) {}

void AppController::OnEvaluate() {
    std::string formatted = util::FormatExpression(state_.expression_input);
    state_.expression_input = formatted;

    EvaluationResult res = calc_.Evaluate(formatted);
    if (res.ok) {
        std::string result_str = util::FormatDouble(res.value);
        state_.result_display = "= " + result_str;
        state_.error_state = false;
        history_ctrl_.OnSaveHistory(formatted, result_str);
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

void AppController::OnClearHistory() { history_ctrl_.OnClearHistory(); }

void AppController::OnOpenVersion() { state_.show_version_modal = true; }

void AppController::OnCloseVersion() { state_.show_version_modal = false; }

const std::vector<std::pair<std::string, std::string>>&
AppController::GetHistory() const {
    return history_ctrl_.GetHistory();
}
