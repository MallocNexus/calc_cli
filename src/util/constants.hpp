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

// ── Exchange Rate API ───────────────────────────────────────────────────────
constexpr std::string_view kFrankFurterApiBaseUrl             = "https://api.frankfurter.dev/v2/rate/";
constexpr int              kFrankFurterApiCacheTtlSeconds     = 86400;
constexpr int              kFrankFurterApiTimeoutMs           = 5000;
constexpr int              kFrankFurterApiHttpStatusOk        = 200;
constexpr std::string_view kFrankFurterApiRateJsonKey         = "rate";
constexpr double           kFrankFurterApiInvalidRateSentinel = 0.0;

// ── Database: Exchange Rate Table ───────────────────────────────────────────
constexpr std::string_view kDbExchangeTable          = "exchange_rates";
constexpr std::string_view kDbExchangeColBase        = "base_currency";
constexpr std::string_view kDbExchangeColQuote       = "quote_currency";
constexpr std::string_view kDbExchangeColRate        = "rate";
constexpr std::string_view kDbExchangeColLastUpdated = "last_updated";

// ── Database: History Table ─────────────────────────────────────────────────
constexpr std::string_view kDbHistoryTable           = "history";
constexpr std::string_view kDbHistoryColExpression   = "expression";
constexpr std::string_view kDbHistoryColResult       = "result";
constexpr std::string_view kDbHistoryColTimestamp    = "timestamp";
// ── Parser ──────────────────────────────────────────────────────────────────
constexpr std::string_view kParserExchangeKeyword    = "exchange";
constexpr std::size_t      kParserExchangeKeywordLen = kParserExchangeKeyword.size();

}  // namespace calc_cli
