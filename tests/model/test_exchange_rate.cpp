#include <catch2/catch_test_macros.hpp>
#include "model/exchange_rate.hpp"

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
