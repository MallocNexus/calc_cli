#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/calculator.hpp"
#include <string>
#include <stdexcept>

using Catch::Matchers::WithinRel;

TEST_CASE("Parser - exchange function with mock resolver", "[parser]") {
    auto mock_resolver = [](const std::string& base, const std::string& quote) {
        if (base == "AUD" && quote == "USD") return 0.70;
        throw std::runtime_error("Unsupported currency pair");
    };

    Calculator calc;
    
    SECTION("Basic exchange lookup") {
        auto res = calc.Evaluate("exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(0.70, 1e-9));
    }

    SECTION("Basic exchange lookup with implicit multiplication") {
        auto res = calc.Evaluate("100 exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(70.0, 1e-9));
    }

    SECTION("Expression-wide suffix precedence") {
        auto res = calc.Evaluate("2 + 4 + 7 * 9 exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(48.3, 1e-9)); // (2 + 4 + 63) * 0.70 = 69 * 0.70 = 48.3
    }

    SECTION("Prefix / term integration") {
        auto res = calc.Evaluate("5 + exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(5.70, 1e-9));
    }
}

TEST_CASE("Parser — Exchange keyword validation", "[parser][constants]") {
    auto mock_resolver = [](const std::string& base, const std::string& quote) {
        if (base == "AUD" && quote == "USD") return 0.70;
        throw std::runtime_error("Unsupported currency pair");
    };
    Calculator calc;

    SECTION("ExchangeKeywordRecognised") {
        auto res = calc.Evaluate("exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(0.70, 1e-9));
    }

    SECTION("PartialKeywordRejected") {
        auto res = calc.Evaluate("exchang(AUD, USD)", mock_resolver);
        REQUIRE_FALSE(res.ok);
    }
}
