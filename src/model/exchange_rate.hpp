#pragma once

#include <string>
#include <sqlite3.h>

struct CachedRate {
    double rate;
    long long timestamp; // Unix timestamp in seconds
};

class ExchangeRate {
  public:
    explicit ExchangeRate(const std::string& db_path);
    ~ExchangeRate();

    // Disable copy
    ExchangeRate(const ExchangeRate&) = delete;
    ExchangeRate& operator=(const ExchangeRate&) = delete;

    bool Initialize();

    // Retrieve cached rate if it exists
    bool GetCachedRate(const std::string& base, const std::string& quote, CachedRate& out_rate);

    // Save/Update rate in the database
    bool SaveRate(const std::string& base, const std::string& quote, double rate);

  private:
    std::string db_path_;
    sqlite3* db_ = nullptr;

    bool ExecuteQuery(const std::string& sql);
};
