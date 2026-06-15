#pragma once

#include <string>
#include <vector>
#include <utility>
#include <sqlite3.h>

class HistoryRepository {
  public:
    explicit HistoryRepository(const std::string& db_path);
    ~HistoryRepository();

    // Disable copy constructors
    HistoryRepository(const HistoryRepository&) = delete;
    HistoryRepository& operator=(const HistoryRepository&) = delete;

    // Load history from the database into the memory cache
    bool Initialize();

    // Save a new calculation and append to memory cache
    bool Add(const std::string& expression, const std::string& result);

    // Delete all records from the database and clear memory cache
    bool Clear();

    // Get read-only access to cached history
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    std::string db_path_;
    sqlite3* db_ = nullptr;
    std::vector<std::pair<std::string, std::string>> cache_;

    bool ExecuteQuery(const std::string& sql);
};
