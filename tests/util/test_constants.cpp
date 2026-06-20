#include <catch2/catch_test_macros.hpp>

#include "util/constants.hpp"

// ── Phase 1: File System Constants ─────────────────────────────────────────

TEST_CASE("FileSysConstants — environment variable names", "[constants][fs]") {
    REQUIRE(calc_cli::kFileSysEnvHome == "HOME");
    REQUIRE(calc_cli::kFileSysEnvUserProfile == "USERPROFILE");
}

TEST_CASE("FileSysConstants — app data paths are well-formed", "[constants][fs]") {
    // Subdir must start with '/' to concatenate correctly after $HOME
    REQUIRE(calc_cli::kFileSysAppDataSubdir.front() == '/');
    // Fallback dir must NOT start with '/' (relative path)
    REQUIRE(calc_cli::kFileSysAppDataFallbackDir.front() != '/');
}

TEST_CASE("FileSysConstants — database filenames are well-formed", "[constants][fs]") {
    // Both must start with '/' to concatenate correctly after the data dir
    REQUIRE(calc_cli::kFileSysHistoryDbFilename.front() == '/');
    REQUIRE(calc_cli::kFileSysExchangeRateDbFilename.front() == '/');

    // Both must end with '.db' (C++17-compatible suffix check)
    auto ends_with = [](std::string_view sv, std::string_view suffix) {
        return sv.size() >= suffix.size() &&
               sv.compare(sv.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    REQUIRE(ends_with(calc_cli::kFileSysHistoryDbFilename, ".db"));
    REQUIRE(ends_with(calc_cli::kFileSysExchangeRateDbFilename, ".db"));

    // Filenames must be distinct
    REQUIRE(calc_cli::kFileSysHistoryDbFilename != calc_cli::kFileSysExchangeRateDbFilename);
}

TEST_CASE("FileSysConstants — path concatenation produces correct full paths", "[constants][fs]") {
    const std::string home = "/home/user";

    std::string history_path = home
                             + std::string(calc_cli::kFileSysAppDataSubdir)
                             + std::string(calc_cli::kFileSysHistoryDbFilename);
    REQUIRE(history_path == "/home/user/.calc_cli/calc_history.db");

    std::string exchange_path = home
                              + std::string(calc_cli::kFileSysAppDataSubdir)
                              + std::string(calc_cli::kFileSysExchangeRateDbFilename);
    REQUIRE(exchange_path == "/home/user/.calc_cli/exchange_rate.db");
}

// ── Phase 2: Exchange Rate API Constants ────────────────────────────────────

TEST_CASE("FrankFurterApiConstants — base URL is well-formed", "[constants][api]") {
    auto starts_with = [](std::string_view sv, std::string_view prefix) {
        return sv.size() >= prefix.size() &&
               sv.compare(0, prefix.size(), prefix) == 0;
    };
    auto ends_with = [](std::string_view sv, std::string_view suffix) {
        return sv.size() >= suffix.size() &&
               sv.compare(sv.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    REQUIRE(starts_with(calc_cli::kFrankFurterApiBaseUrl, "https://"));
    REQUIRE(ends_with(calc_cli::kFrankFurterApiBaseUrl, "/"));
}

TEST_CASE("FrankFurterApiConstants — cache TTL equals 24 hours in seconds", "[constants][api]") {
    REQUIRE(calc_cli::kFrankFurterApiCacheTtlSeconds == 86400);
    REQUIRE(calc_cli::kFrankFurterApiCacheTtlSeconds == 24 * 60 * 60);
}

TEST_CASE("FrankFurterApiConstants — timeout is positive", "[constants][api]") {
    REQUIRE(calc_cli::kFrankFurterApiTimeoutMs > 0);
}

TEST_CASE("FrankFurterApiConstants — HTTP status OK is 200", "[constants][api]") {
    REQUIRE(calc_cli::kFrankFurterApiHttpStatusOk == 200);
}

TEST_CASE("FrankFurterApiConstants — rate JSON key is correct", "[constants][api]") {
    REQUIRE(calc_cli::kFrankFurterApiRateJsonKey == "rate");
}

TEST_CASE("FrankFurterApiConstants — invalid rate sentinel is zero", "[constants][api]") {
    REQUIRE(calc_cli::kFrankFurterApiInvalidRateSentinel == 0.0);
}

// ── Phase 3: Database Constants ─────────────────────────────────────────────

TEST_CASE("DbConstants — exchange table name is correct", "[constants][db]") {
    REQUIRE(calc_cli::kDbExchangeTable == "exchange_rates");
}

TEST_CASE("DbConstants — exchange column names are non-empty and distinct", "[constants][db]") {
    REQUIRE_FALSE(calc_cli::kDbExchangeColBase.empty());
    REQUIRE_FALSE(calc_cli::kDbExchangeColQuote.empty());
    REQUIRE_FALSE(calc_cli::kDbExchangeColRate.empty());
    REQUIRE_FALSE(calc_cli::kDbExchangeColLastUpdated.empty());

    // All 4 must be distinct
    REQUIRE(calc_cli::kDbExchangeColBase        != calc_cli::kDbExchangeColQuote);
    REQUIRE(calc_cli::kDbExchangeColBase        != calc_cli::kDbExchangeColRate);
    REQUIRE(calc_cli::kDbExchangeColBase        != calc_cli::kDbExchangeColLastUpdated);
    REQUIRE(calc_cli::kDbExchangeColQuote       != calc_cli::kDbExchangeColRate);
    REQUIRE(calc_cli::kDbExchangeColQuote       != calc_cli::kDbExchangeColLastUpdated);
    REQUIRE(calc_cli::kDbExchangeColRate        != calc_cli::kDbExchangeColLastUpdated);
}

TEST_CASE("DbConstants — history table name is correct", "[constants][db]") {
    REQUIRE(calc_cli::kDbHistoryTable == "history");
}

TEST_CASE("DbConstants — history column names are non-empty and distinct", "[constants][db]") {
    REQUIRE_FALSE(calc_cli::kDbHistoryColExpression.empty());
    REQUIRE_FALSE(calc_cli::kDbHistoryColResult.empty());
    REQUIRE_FALSE(calc_cli::kDbHistoryColTimestamp.empty());

    REQUIRE(calc_cli::kDbHistoryColExpression != calc_cli::kDbHistoryColResult);
    REQUIRE(calc_cli::kDbHistoryColExpression != calc_cli::kDbHistoryColTimestamp);
    REQUIRE(calc_cli::kDbHistoryColResult     != calc_cli::kDbHistoryColTimestamp);
}

TEST_CASE("DbConstants — no column name collisions across both tables", "[constants][db]") {
    // Exchange and history column names must all be unique across both tables
    std::string_view exchange_cols[] = {
        calc_cli::kDbExchangeColBase,
        calc_cli::kDbExchangeColQuote,
        calc_cli::kDbExchangeColRate,
        calc_cli::kDbExchangeColLastUpdated,
    };
    std::string_view history_cols[] = {
        calc_cli::kDbHistoryColExpression,
        calc_cli::kDbHistoryColResult,
        calc_cli::kDbHistoryColTimestamp,
    };
    for (auto ec : exchange_cols) {
        for (auto hc : history_cols) {
            REQUIRE(ec != hc);
        }
    }
}

// ── Phase 4: Parser Constants ───────────────────────────────────────────────

TEST_CASE("ParserConstants — keyword value is correct", "[constants][parser]") {
    REQUIRE(calc_cli::kParserExchangeKeyword == "exchange");
}

TEST_CASE("ParserConstants — keyword length is derived correctly", "[constants][parser]") {
    REQUIRE(calc_cli::kParserExchangeKeywordLen == calc_cli::kParserExchangeKeyword.size());
}

TEST_CASE("ParserConstants — keyword length is exactly 8", "[constants][parser]") {
    REQUIRE(calc_cli::kParserExchangeKeywordLen == 8);
}
