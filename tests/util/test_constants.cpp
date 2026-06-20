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
