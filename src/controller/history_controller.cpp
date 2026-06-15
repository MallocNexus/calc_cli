#include "controller/history_controller.hpp"

HistoryController::HistoryController(HistoryRepository& repo)
    : repo_(repo) {}

void HistoryController::OnSaveHistory(const std::string& expression, const std::string& result) {
    repo_.Add(expression, result);
}

void HistoryController::OnClearHistory() {
    repo_.Clear();
}

const std::vector<std::pair<std::string, std::string>>& HistoryController::GetHistory() const {
    return repo_.GetHistory();
}
