#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/exchange_rate.hpp"
#include "controller/exchange_rate_controller.hpp"
#include "util/constants.hpp"
#include <ctime>
#include <sqlite3.h>
#include <filesystem>

using Catch::Matchers::WithinRel;

TEST_CASE("ExchangeRateController — caching and fallback logic", "[exchange_rate_controller]") {
    std::string db_path = "test_exchange_rate_ctrl.db";
    std::filesystem::remove(db_path);

    {
        ExchangeRate repo(db_path);
        REQUIRE(repo.Initialize());
        ExchangeRateController controller(repo);

        SECTION("Fresh cache hit returns rate without network fetch") {
            double test_rate = 1.2345;
            REQUIRE(repo.SaveRate("EUR", "USD", test_rate));

            // Should hit cache and return test_rate immediately
            double rate = controller.GetRate("EUR", "USD");
            REQUIRE_THAT(rate, WithinRel(test_rate, 1e-9));
        }
    }

    {
        ExchangeRate repo(db_path);
        REQUIRE(repo.Initialize());
        ExchangeRateController controller(repo);

        SECTION("Stale cache falls back to cached rate if API request fails") {
            double test_rate = 0.9876;
            REQUIRE(repo.SaveRate("XYZ", "ABC", test_rate));

            // Manually update the database to set last_updated to a stale value (> 24 hours ago)
            sqlite3* db = nullptr;
            REQUIRE(sqlite3_open(db_path.c_str(), &db) == SQLITE_OK);
            std::string sql = "UPDATE exchange_rates SET last_updated = last_updated - 90000 WHERE base_currency = 'XYZ' AND quote_currency = 'ABC';";
            char* err = nullptr;
            REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
            sqlite3_close(db);

            // Fetching a stale rate for invalid currencies will trigger cpr API request which fails,
            // causing the controller to fall back to the stale rate we stored in the database.
            double rate = controller.GetRate("XYZ", "ABC");
            REQUIRE_THAT(rate, WithinRel(test_rate, 1e-9));
        }
    }

    std::filesystem::remove(db_path);
}

TEST_CASE("ExchangeRateController — cache TTL boundary uses kFrankFurterApiCacheTtlSeconds", "[exchange_rate_controller][constants]") {
    std::string db_path = "test_exchange_rate_ctrl_ttl.db";
    std::filesystem::remove(db_path);

    ExchangeRate repo(db_path);
    REQUIRE(repo.Initialize());
    ExchangeRateController controller(repo);

    double test_rate = 2.5;
    REQUIRE(repo.SaveRate("GBP", "USD", test_rate));

    SECTION("Rate saved less than kFrankFurterApiCacheTtlSeconds ago is returned from cache") {
        // Rate was just saved — timestamp is now, well within TTL
        double rate = controller.GetRate("GBP", "USD");
        REQUIRE_THAT(rate, WithinRel(test_rate, 1e-9));
    }

    SECTION("Rate older than kFrankFurterApiCacheTtlSeconds triggers re-fetch fallback to stale cache") {
        // Use invalid currencies so the API call is guaranteed to fail
        double stale_rate = 9.9;
        REQUIRE(repo.SaveRate("ZZZ", "QQQ", stale_rate));

        // Age the timestamp beyond the TTL using kFrankFurterApiCacheTtlSeconds
        sqlite3* db = nullptr;
        REQUIRE(sqlite3_open(db_path.c_str(), &db) == SQLITE_OK);
        std::string sql = "UPDATE exchange_rates SET last_updated = last_updated - " +
                          std::to_string(calc_cli::kFrankFurterApiCacheTtlSeconds + 1) +
                          " WHERE base_currency = 'ZZZ' AND quote_currency = 'QQQ';";
        char* err = nullptr;
        REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
        sqlite3_close(db);

        // Stale + invalid currencies → API fails → falls back to stale cached rate
        double rate = controller.GetRate("ZZZ", "QQQ");
        REQUIRE_THAT(rate, WithinRel(stale_rate, 1e-9));
    }

    std::filesystem::remove(db_path);
}
