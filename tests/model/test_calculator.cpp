#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "model/calculator.hpp"

using Catch::Matchers::WithinRel;

TEST_CASE("Calculator — basic arithmetic", "[calculator]") {
    Calculator calc;

    SECTION("Addition") {
        auto res = calc.Evaluate("2 + 3");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(5.0, 1e-9));
    }

    SECTION("Subtraction") {
        auto res = calc.Evaluate("10 - 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(6.0, 1e-9));
    }

    SECTION("Multiplication") {
        auto res = calc.Evaluate("3 * 7");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(21.0, 1e-9));
    }

    SECTION("Division") {
        auto res = calc.Evaluate("15 / 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(3.75, 1e-9));
    }
}

TEST_CASE("Calculator — operator precedence", "[calculator]") {
    Calculator calc;

    SECTION("Multiplication before addition") {
        auto res = calc.Evaluate("2 + 3 * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(14.0, 1e-9));
    }

    SECTION("Parentheses override precedence") {
        auto res = calc.Evaluate("(2 + 3) * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(20.0, 1e-9));
    }

    SECTION("Nested parentheses") {
        auto res = calc.Evaluate("3 + 4 * (2 - 1)");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(7.0, 1e-9));
    }
}

TEST_CASE("Calculator — unary minus and floats", "[calculator]") {
    Calculator calc;

    SECTION("Unary minus") {
        auto res = calc.Evaluate("-3 + 1");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-2.0, 1e-9));
    }

    SECTION("Floating point input") {
        auto res = calc.Evaluate("1.5 + 2.5");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(4.0, 1e-9));
    }

    SECTION("Negative result") {
        auto res = calc.Evaluate("3 - 10");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-7.0, 1e-9));
    }
}

TEST_CASE("Calculator — error cases", "[calculator]") {
    Calculator calc;

    SECTION("Division by zero") {
        auto res = calc.Evaluate("5 / 0");
        REQUIRE_FALSE(res.ok);
        REQUIRE_FALSE(res.error.empty());
    }

    SECTION("Empty expression") {
        auto res = calc.Evaluate("");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Invalid characters") {
        auto res = calc.Evaluate("abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Trailing garbage") {
        auto res = calc.Evaluate("3 + 4 abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Mismatched parenthesis") {
        auto res = calc.Evaluate("(3 + 4");
        REQUIRE_FALSE(res.ok);
    }
}
