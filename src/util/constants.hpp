#pragma once

#include <cstddef>
#include <string_view>

namespace calc_cli {

// Constants are added here phase by phase.
// See docs/future-refactoring-ideas.md for deferred constants.

// ── File System ────────────────────────────────────────────────────────────
constexpr std::string_view kFileSysEnvHome                = "HOME";
constexpr std::string_view kFileSysEnvUserProfile         = "USERPROFILE";
constexpr std::string_view kFileSysAppDataSubdir          = "/.calc_cli";
constexpr std::string_view kFileSysAppDataFallbackDir     = ".calc_cli";
constexpr std::string_view kFileSysHistoryDbFilename      = "/calc_history.db";
constexpr std::string_view kFileSysExchangeRateDbFilename = "/exchange_rate.db";

}  // namespace calc_cli
