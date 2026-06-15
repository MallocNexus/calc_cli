#include <catch2/catch_test_macros.hpp>
#include "util/formatting.hpp"

TEST_CASE("Util — FormatExpression", "[util]") {
    SECTION("Basic spacing") {
        REQUIRE(util::FormatExpression("2+2") == "2 + 2");
        REQUIRE(util::FormatExpression(" 2  +   2 ") == "2 + 2");
    }

    SECTION("Parentheses") {
        REQUIRE(util::FormatExpression("3*(4-1)") == "3 * (4 - 1)");
        REQUIRE(util::FormatExpression("( 2 + 2 )") == "(2 + 2)");
        REQUIRE(util::FormatExpression(" (2)+(3) ") == "(2) + (3)");
    }

    SECTION("Unary minus and plus") {
        REQUIRE(util::FormatExpression("-5 + -2") == "-5 + -2");
        REQUIRE(util::FormatExpression("+5 * -3") == "+5 * -3");
        REQUIRE(util::FormatExpression("-(2+2)") == "-(2 + 2)");
        REQUIRE(util::FormatExpression("- -2") == "- -2");
    }

    SECTION("Exchange function") {
        REQUIRE(util::FormatExpression("100exchange(AUD,USD)") == "100 exchange(AUD, USD)");
        REQUIRE(util::FormatExpression("2+2 exchange(AUD,USD)") == "2 + 2 exchange(AUD, USD)");
        REQUIRE(util::FormatExpression("exchange(AUD,USD)") == "exchange(AUD, USD)");
        REQUIRE(util::FormatExpression("exchange(AUD,USD)+5") == "exchange(AUD, USD) + 5");
    }
}
