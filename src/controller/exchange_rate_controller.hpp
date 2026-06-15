#pragma once

#include "model/exchange_rate.hpp"
#include <string>

class ExchangeRateController {
  public:
    explicit ExchangeRateController(ExchangeRate& model);

    // Resolves exchange rate. Checks cache first; if missing or > 24 hours old,
    // fetches from Frankfurter API via cpr and saves to cache.
    double GetRate(const std::string& base, const std::string& quote);

  private:
    ExchangeRate& model_;

    double FetchFromAPI(const std::string& base, const std::string& quote);
};
