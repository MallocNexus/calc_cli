# CLI Mode & Expression Spacing Plan

This document outlines two enhancements to the calculator:
1. Adding a headless CLI mode (`calc_cli -expr "2+2"`).
2. Auto-formatting expressions so that both the CLI output and UI history display cleanly spaced operators (e.g., `2+2` becomes `2 + 2`).

---

## 1. Expression Formatting (Spacing)

Currently, the user's exact input is saved to history (e.g., `2+2`). We need a utility that normalizes the spacing of mathematical expressions.

### 1.1 `util::FormatExpression`
We will add a new function to `src/util/formatting.hpp`:

```cpp
std::string FormatExpression(const std::string& input);
```

**Rules for formatting:**
- All extra whitespace is removed.
- Binary operators (`+`, `-`, `*`, `/`) will have exactly one space on each side.
- Parentheses `(` and `)` will hug the content inside them, and have spaces outside (e.g., `2 * (3 + 1)`).
- **Unary Minus:** We must distinguish between a subtraction operator (`3 - 2`) and a unary minus (`-2`). A minus is unary if it is the first character, or if it immediately follows another operator or a left parenthesis `(`. Unary minus will attach directly to the number without a space.

### 1.2 Integration in `AppController`
In `AppController::OnEvaluate()`:
If evaluation is successful, we format the expression. 
We will update `state_.expression_input` to be the formatted string. This provides immediate visual feedback to the user, cleaning up their input box.
Because `Calculator::Evaluate()` already records the string you pass to it in the history, we will pass the *formatted* string to `Evaluate()`.

*Wait, `Calculator::Evaluate()` does the history recording. If we format it before calling `Evaluate()`, the model will automatically record the formatted version!*

**Updated flow in `AppController::OnEvaluate()`:**
```cpp
// Format before evaluating
std::string formatted = util::FormatExpression(state_.expression_input);
state_.expression_input = formatted; // Clean up the UI input field

EvaluationResult res = calc_.Evaluate(formatted);
// ... handle success/error
```

---

## 2. CLI Headless Mode

The user wants to be able to run:
```bash
calc_cli -expr "2+2"
```
And see the output:
```text
2 + 2 = 4
```

### 2.1 Updating `main.cpp`
We will parse `argc` and `argv` before initializing FTXUI. 

If the arguments match the CLI mode signature, we will:
1. Instantiate `Calculator`.
2. Read the expression string from `argv[2]`.
3. Format the expression using `util::FormatExpression`.
4. Call `calc.Evaluate()`.
5. Print the formatted output to `std::cout` (or `std::cerr` on error).
6. Exit immediately (bypassing FTXUI completely).

```cpp
int main(int argc, char* argv[]) {
    // 1. Check for CLI mode
    if (argc == 3 && std::string(argv[1]) == "-expr") {
        std::string expr = argv[2];
        std::string formatted = util::FormatExpression(expr);
        
        Calculator calc;
        EvaluationResult res = calc.Evaluate(formatted);
        
        if (res.ok) {
            std::cout << formatted << " = " << util::FormatDouble(res.value) << "\n";
            return EXIT_SUCCESS;
        } else {
            std::cerr << "Error: " << res.error << "\n";
            return EXIT_FAILURE;
        }
    }

    // 2. Otherwise, start FTXUI mode
    // ... existing FTXUI initialization ...
}
```

---

## 3. Implementation Checklist

- [ ] **`src/util/formatting.hpp` & `.cpp`**: 
  - Add `FormatExpression(const std::string& input)`.
  - Implement a tokenizing loop that correctly handles unary minus and spacing.
- [ ] **`tests/test_main.cpp`**: 
  - Add tests for `FormatExpression` to ensure `2+2`, `3 * ( 4-1)`, and `-5 + -2` are spaced correctly.
- [ ] **`src/controller/app_controller.cpp`**: 
  - Modify `OnEvaluate` to format the expression *before* calling `calc_.Evaluate()`, and update `state_.expression_input` with the clean string.
- [ ] **`src/main.cpp`**: 
  - Add standard `<iostream>` include.
  - Add `argc`, `argv` parsing at the top of `main()`.
  - Implement the headless execution path.
- [ ] **Build & Test**:
  - `cmake --build build && ./build/run_tests`
  - Manual verification: `./build/calc_cli -expr "2+2"` and observe output.
