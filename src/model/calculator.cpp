#include "model/calculator.hpp"
#include "model/parser.hpp"
#include "util/formatting.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

// ---------------------------------------------------------------------------
// Calculator public interface
// ---------------------------------------------------------------------------

EvaluationResult Calculator::Evaluate(const std::string& expression) {
    if (expression.empty()) {
        return {false, 0.0, "Empty expression"};
    }

    try {
        model::internal::Parser parser(expression);
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

        history_.emplace_back(expression, util::FormatDouble(value));
        return {true, value, ""};

    } catch (const std::exception& e) {
        return {false, 0.0, e.what()};
    }
}

void Calculator::ClearHistory() { history_.clear(); }

const std::vector<std::pair<std::string, std::string>>& Calculator::GetHistory() const {
    return history_;
}
