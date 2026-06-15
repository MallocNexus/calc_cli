#include "model/exchange_rate.hpp"
#include <iostream>
#include <filesystem>
#include <ctime>

ExchangeRate::ExchangeRate(const std::string& db_path)
    : db_path_(db_path) {}

ExchangeRate::~ExchangeRate() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool ExchangeRate::Initialize() {
    try {
        std::filesystem::path p(db_path_);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to create database directory: " << e.what() << std::endl;
        return false;
    }

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    std::string create_table_sql =
        "CREATE TABLE IF NOT EXISTS exchange_rates ("
        "base_currency TEXT NOT NULL,"
        "quote_currency TEXT NOT NULL,"
        "rate REAL NOT NULL,"
        "last_updated INTEGER NOT NULL,"
        "PRIMARY KEY (base_currency, quote_currency)"
        ");";

    return ExecuteQuery(create_table_sql);
}

bool ExchangeRate::GetCachedRate(const std::string& base, const std::string& quote, CachedRate& out_rate) {
    if (!db_) return false;

    std::string select_sql = "SELECT rate, last_updated FROM exchange_rates WHERE base_currency = ? AND quote_currency = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, select_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare select query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, base.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, quote.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_rate.rate = sqlite3_column_double(stmt, 0);
        out_rate.timestamp = sqlite3_column_int64(stmt, 1);
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool ExchangeRate::SaveRate(const std::string& base, const std::string& quote, double rate) {
    if (!db_) return false;

    std::string insert_sql = "INSERT OR REPLACE INTO exchange_rates (base_currency, quote_currency, rate, last_updated) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, insert_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare insert/replace query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    long long current_time = std::time(nullptr);
    sqlite3_bind_text(stmt, 1, base.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, quote.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, rate);
    sqlite3_bind_int64(stmt, 4, current_time);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Error: Failed to save exchange rate: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    return true;
}

bool ExchangeRate::ExecuteQuery(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}
