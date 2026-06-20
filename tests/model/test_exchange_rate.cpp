#include <catch2/catch_test_macros.hpp>
#include "model/exchange_rate.hpp"
#include "util/constants.hpp"
#include <sqlite3.h>
#include <filesystem>

TEST_CASE("ExchangeRate Cache Model", "[exchange_rate]") {
    ExchangeRate repo(":memory:");
    REQUIRE(repo.Initialize());

    SECTION("Get cached rate for missing pair returns false") {
        CachedRate rate;
        REQUIRE_FALSE(repo.GetCachedRate("AUD", "USD", rate));
    }

    SECTION("Save and retrieve rate") {
        REQUIRE(repo.SaveRate("AUD", "USD", 0.70));
        CachedRate rate;
        REQUIRE(repo.GetCachedRate("AUD", "USD", rate));
        REQUIRE(rate.rate == 0.70);
        REQUIRE(rate.timestamp > 0);
    }
}

TEST_CASE("ExchangeRate — Table schema matches constants", "[exchange_rate][constants]") {
    std::string db_path = "test_exchange_rate_schema.db";
    std::filesystem::remove(db_path);

    {
        ExchangeRate repo(db_path);
        REQUIRE(repo.Initialize());
    }

    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(db_path.c_str(), &db) == SQLITE_OK);

    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + std::string(calc_cli::kDbExchangeTable) + "';";
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    const unsigned char* table_name = sqlite3_column_text(stmt, 0);
    REQUIRE(table_name != nullptr);
    REQUIRE(std::string(reinterpret_cast<const char*>(table_name)) == calc_cli::kDbExchangeTable);
    sqlite3_finalize(stmt);

    std::string pragma_sql = "PRAGMA table_info(" + std::string(calc_cli::kDbExchangeTable) + ");";
    REQUIRE(sqlite3_prepare_v2(db, pragma_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK);

    bool has_base = false;
    bool has_quote = false;
    bool has_rate = false;
    bool has_last_updated = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* col_name = sqlite3_column_text(stmt, 1);
        if (col_name) {
            std::string name(reinterpret_cast<const char*>(col_name));
            if (name == calc_cli::kDbExchangeColBase) has_base = true;
            else if (name == calc_cli::kDbExchangeColQuote) has_quote = true;
            else if (name == calc_cli::kDbExchangeColRate) has_rate = true;
            else if (name == calc_cli::kDbExchangeColLastUpdated) has_last_updated = true;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(db_path);

    REQUIRE(has_base);
    REQUIRE(has_quote);
    REQUIRE(has_rate);
    REQUIRE(has_last_updated);
}
