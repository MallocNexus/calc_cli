#include <catch2/catch_test_macros.hpp>
#include "model/history_repository.hpp"
#include "util/constants.hpp"
#include <sqlite3.h>
#include <filesystem>

TEST_CASE("HistoryRepository — database operation", "[history]") {
    HistoryRepository repo(":memory:");
    REQUIRE(repo.Initialize());

    SECTION("History is empty initially") {
        REQUIRE(repo.GetHistory().empty());
    }

    SECTION("Successful evaluations are recorded") {
        REQUIRE(repo.Add("1 + 1", "2"));
        REQUIRE(repo.Add("2 * 3", "6"));
        REQUIRE(repo.GetHistory().size() == 2);
        REQUIRE(repo.GetHistory()[0].first == "1 + 1");
        REQUIRE(repo.GetHistory()[0].second == "2");
        REQUIRE(repo.GetHistory()[1].first == "2 * 3");
        REQUIRE(repo.GetHistory()[1].second == "6");
    }

    SECTION("Clear empties the record") {
        REQUIRE(repo.Add("1 + 1", "2"));
        REQUIRE(repo.Clear());
        REQUIRE(repo.GetHistory().empty());
    }
}

TEST_CASE("HistoryRepository — table schema matches constants", "[history][constants]") {
    std::string db_path = "test_history_repo_schema.db";
    std::filesystem::remove(db_path);

    {
        HistoryRepository repo(db_path);
        REQUIRE(repo.Initialize());
    }

    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(db_path.c_str(), &db) == SQLITE_OK);

    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + std::string(calc_cli::kDbHistoryTable) + "';";
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    const unsigned char* table_name = sqlite3_column_text(stmt, 0);
    REQUIRE(table_name != nullptr);
    REQUIRE(std::string(reinterpret_cast<const char*>(table_name)) == calc_cli::kDbHistoryTable);
    sqlite3_finalize(stmt);

    std::string pragma_sql = "PRAGMA table_info(" + std::string(calc_cli::kDbHistoryTable) + ");";
    REQUIRE(sqlite3_prepare_v2(db, pragma_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK);

    bool has_expression = false;
    bool has_result = false;
    bool has_timestamp = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* col_name = sqlite3_column_text(stmt, 1);
        if (col_name) {
            std::string name(reinterpret_cast<const char*>(col_name));
            if (name == calc_cli::kDbHistoryColExpression) has_expression = true;
            else if (name == calc_cli::kDbHistoryColResult) has_result = true;
            else if (name == calc_cli::kDbHistoryColTimestamp) has_timestamp = true;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(db_path);

    REQUIRE(has_expression);
    REQUIRE(has_result);
    REQUIRE(has_timestamp);
}
