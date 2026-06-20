#include "controller/exchange_rate_controller.hpp"
#include "util/constants.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <ctime>
#include <stdexcept>
#include <iostream>

ExchangeRateController::ExchangeRateController(ExchangeRate& model)
    : model_(model) {}

double ExchangeRateController::GetRate(const std::string& base, const std::string& quote) {
    CachedRate cached;
    long long current_time = std::time(nullptr);

    // If cache exists and is fresh (< kFrankFurterApiCacheTtlSeconds seconds / 24 hours)
    if (model_.GetCachedRate(base, quote, cached)) {
        if (current_time - cached.timestamp < calc_cli::kFrankFurterApiCacheTtlSeconds) {
            return cached.rate;
        }
    }

    // Cache miss or stale -> Fetch from API
    try {
        double rate = FetchFromAPI(base, quote);
        model_.SaveRate(base, quote, rate);
        return rate;
    } catch (const std::exception& e) {
        // Fallback to stale cache if API call fails (offline support)
        if (cached.rate > calc_cli::kFrankFurterApiInvalidRateSentinel) {
            return cached.rate;
        }
        throw; // rethrow if we have no fallback rate
    }
}

double ExchangeRateController::FetchFromAPI(const std::string& base, const std::string& quote) {
    std::string url = std::string(calc_cli::kFrankFurterApiBaseUrl) + base + "/" + quote;

    // Execute HTTP GET request using CPR
    cpr::Response r = cpr::Get(cpr::Url{url}, cpr::Timeout{calc_cli::kFrankFurterApiTimeoutMs}); // 5-second timeout
    if (r.status_code != calc_cli::kFrankFurterApiHttpStatusOk) {
        throw std::runtime_error("Network request failed (status " + 
                                  std::to_string(r.status_code) + "): " + r.error.message);
    }

    try {
        auto json = nlohmann::json::parse(r.text);
        if (!json.contains(calc_cli::kFrankFurterApiRateJsonKey) || !json[std::string(calc_cli::kFrankFurterApiRateJsonKey)].is_number()) {
            throw std::runtime_error("Invalid API response format");
        }
        return json[std::string(calc_cli::kFrankFurterApiRateJsonKey)].get<double>();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse exchange rate: " + std::string(e.what()));
    }
}
