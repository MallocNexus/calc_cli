#pragma once

#include "model/history_repository.hpp"
#include <string>
#include <vector>
#include <utility>

class HistoryController {
  public:
    explicit HistoryController(HistoryRepository& repo);

    // Save a new calculation to database and cache
    void OnSaveHistory(const std::string& expression, const std::string& result);

    // Clear all history from database and cache
    void OnClearHistory();

    // Read-only access to history cache for the view to render
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    HistoryRepository& repo_;
};
