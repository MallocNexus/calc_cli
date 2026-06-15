#pragma once

#include <string>
#include <utility>
#include <vector>

// Result of a single expression evaluation.
// On success: ok == true and value holds the computed result.
// On failure: ok == false and error holds a human-readable message.
struct EvaluationResult {
    bool ok;
    double value;
    std::string error;
};

// Calculator evaluates infix arithmetic expressions and tracks history.
// This class has NO FTXUI dependency and can be tested in complete isolation.
class Calculator {
  public:
    // Evaluates `expression` and appends to history on success.
    EvaluationResult Evaluate(const std::string& expression);

    // Clears the evaluation history.
    void ClearHistory();

    // Returns all past evaluations as (expression, result_string) pairs.
    const std::vector<std::pair<std::string, std::string>>& GetHistory() const;

  private:
    std::vector<std::pair<std::string, std::string>> history_;

    // Internal recursive descent parser.
    EvaluationResult ParseAndEvaluate(const std::string& expr);
};
