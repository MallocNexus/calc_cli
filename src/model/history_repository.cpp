#include "model/history_repository.hpp"
#include "util/constants.hpp"
#include <iostream>
#include <format>

HistoryRepository::HistoryRepository(const std::string& db_path)
    : db_path_(db_path) {}

HistoryRepository::~HistoryRepository() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool HistoryRepository::Initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Create table if it doesn't exist
    std::string create_table_sql = std::format(
        "CREATE TABLE IF NOT EXISTS {} ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "{} TEXT NOT NULL, "
        "{} TEXT NOT NULL, "
        "{} DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        calc_cli::kDbHistoryTable,
        calc_cli::kDbHistoryColExpression,
        calc_cli::kDbHistoryColResult,
        calc_cli::kDbHistoryColTimestamp
    );

    if (!ExecuteQuery(create_table_sql)) {
        return false;
    }

    // Load existing history from database into the cache
    std::string select_sql = std::format(
        "SELECT {}, {} FROM {} ORDER BY id ASC;",
        calc_cli::kDbHistoryColExpression,
        calc_cli::kDbHistoryColResult,
        calc_cli::kDbHistoryTable
    );
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, select_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare select query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    cache_.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* expr_txt = sqlite3_column_text(stmt, 0);
        const unsigned char* res_txt = sqlite3_column_text(stmt, 1);
        
        std::string expression = expr_txt ? reinterpret_cast<const char*>(expr_txt) : "";
        std::string result = res_txt ? reinterpret_cast<const char*>(res_txt) : "";
        
        cache_.emplace_back(expression, result);
    }
    sqlite3_finalize(stmt);

    return true;
}

bool HistoryRepository::Add(const std::string& expression, const std::string& result) {
    if (!db_) return false;

    std::string insert_sql = std::format(
        "INSERT INTO {} ({}, {}) VALUES (?, ?);",
        calc_cli::kDbHistoryTable,
        calc_cli::kDbHistoryColExpression,
        calc_cli::kDbHistoryColResult
    );

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insert_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare insert query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, expression.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, result.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Error: Failed to insert history record: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    cache_.emplace_back(expression, result);
    return true;
}

bool HistoryRepository::Clear() {
    if (!db_) return false;

    std::string delete_sql = std::format("DELETE FROM {};", calc_cli::kDbHistoryTable);
    if (!ExecuteQuery(delete_sql)) {
        return false;
    }

    cache_.clear();
    return true;
}

const std::vector<std::pair<std::string, std::string>>& HistoryRepository::GetHistory() const {
    return cache_;
}

bool HistoryRepository::ExecuteQuery(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}
