#include "service/calculator.hpp"
#include "service/parser.hpp"
#include "util/formatting.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

// ---------------------------------------------------------------------------
// Calculator public interface
// ---------------------------------------------------------------------------

EvaluationResult Calculator::Evaluate(const std::string& expression, RateResolver resolver) {
    if (expression.empty()) {
        return {false, 0.0, "Empty expression"};
    }

    try {
        service::internal::Parser parser(expression, resolver);
        double value = parser.ParseExpr();

        // Reject trailing garbage after a valid expression (e.g. "3 + 4 abc").
        size_t pos = parser.Position();
        while (pos < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[pos]))) {
            ++pos;
        }
        if (pos < expression.size()) {
            std::string msg = "Unexpected character: '";
            msg += expression[pos];
            msg += "'";
            return {false, 0.0, msg};
        }

        if (!std::isfinite(value)) {
            return {false, 0.0, "Result is not finite"};
        }

        return {true, value, ""};

    } catch (const std::exception& e) {
        return {false, 0.0, e.what()};
    }
}
