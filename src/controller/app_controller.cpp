#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "controller/exchange_rate_controller.hpp"
#include "model/calculator.hpp"
#include "util/formatting.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------
// AppController implementation
// All event-handling logic lives here — no FTXUI in this file.
// ---------------------------------------------------------------------------

AppController::AppController(AppState& state, Calculator& calc,
                             HistoryController& history_ctrl,
                             ExchangeRateController& exch_rate_ctrl,
                             std::function<void()> on_quit)
    : state_(state), calc_(calc), history_ctrl_(history_ctrl), exch_rate_ctrl_(exch_rate_ctrl), on_quit_(on_quit) {
    SyncHistoryMenuEntries();
}

void AppController::OnEvaluate() {
    std::string formatted = util::FormatExpression(state_.expression_input);
    state_.expression_input = formatted;

    auto resolver = [this](const std::string& base, const std::string& quote) {
        return exch_rate_ctrl_.GetRate(base, quote);
    };

    EvaluationResult res = calc_.Evaluate(formatted, resolver);
    if (res.ok) {
        std::string result_str = util::FormatDouble(res.value);
        state_.result_display = "= " + result_str;
        state_.error_state = false;
        history_ctrl_.OnSaveHistory(formatted, result_str);
        SyncHistoryMenuEntries();
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

void AppController::OnClearHistory() {
    history_ctrl_.OnClearHistory();
    SyncHistoryMenuEntries();
}

void AppController::OnOpenVersion() { state_.show_version_modal = true; }

void AppController::OnCloseVersion() { state_.show_version_modal = false; }

const std::vector<std::pair<std::string, std::string>>&
AppController::GetHistory() const {
    return history_ctrl_.GetHistory();
}

void AppController::OnUseSelectedHistory() {
    const auto& hist = history_ctrl_.GetHistory();
    if (state_.selected_history_idx >= 0 &&
        state_.selected_history_idx < static_cast<int>(hist.size())) {
        const auto& selected_pair = hist[state_.selected_history_idx];
        state_.expression_input = selected_pair.first;
        state_.cursor_position = static_cast<int>(selected_pair.first.size());
    }
}

void AppController::SyncHistoryMenuEntries() {
    state_.history_menu_entries.clear();
    const auto& hist = history_ctrl_.GetHistory();
    for (const auto& [expr, res] : hist) {
        state_.history_menu_entries.push_back(expr + " = " + res);
    }
    // Default selected index to the last entry (newest calculation)
    if (!state_.history_menu_entries.empty()) {
        state_.selected_history_idx = static_cast<int>(state_.history_menu_entries.size()) - 1;
    } else {
        state_.selected_history_idx = 0;
    }
}
