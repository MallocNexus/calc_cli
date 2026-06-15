# Stdin Pipe Support Plan

This document outlines the approach to support piping expressions into the calculator, e.g., `echo "2+2" | build/calc_cli`.

---

## 1. Goal

Allow `calc_cli` to detect when it's being run as part of a shell pipeline rather than in an interactive terminal. If it detects a pipe, it will read the expression from standard input (`stdin`), calculate it, and exit headless mode — exactly matching the behaviour of the `-expr` argument.

---

## 2. Detecting the Pipe

When a program is run via a pipe (like `echo "..." | calc_cli`), its standard input is redirected to a pipe descriptor rather than a terminal (TTY). 

We can detect this using the POSIX `isatty()` function from `<unistd.h>`:
```cpp
if (!isatty(STDIN_FILENO)) {
    // stdin is a pipe or file redirect!
}
```

If `isatty(STDIN_FILENO)` returns false, we know we should *not* attempt to launch the FTXUI interface (which requires a terminal to read keystrokes) and instead run in headless mode.

---

## 3. Extracting Headless Logic

Currently, the headless evaluation logic (instantiating `Calculator`, formatting, evaluating, and printing) is hardcoded inside the `-expr` `if` block. To avoid duplication, we will extract this into a helper function at the top of `main.cpp`:

```cpp
int RunHeadless(const std::string& expr) {
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
```

---

## 4. Updates to `main()`

We will check for a pipe *before* evaluating command-line arguments. If a pipe is active, we read the expression using `std::getline` or `std::cin >>`.

```cpp
int main(int argc, char* argv[]) {
    // 1. Check for piped input (stdin is not a terminal)
    if (!isatty(STDIN_FILENO)) {
        std::string expr;
        std::getline(std::cin, expr);
        
        // Strip any trailing newlines or spaces if necessary, though
        // util::FormatExpression already skips whitespace.
        
        if (!expr.empty()) {
            return RunHeadless(expr);
        }
    }

    // 2. Check for explicit command-line argument
    if (argc == 3 && std::string(argv[1]) == "-expr") {
        return RunHeadless(argv[2]);
    }

    // 3. Fallback to Interactive TUI mode
    using namespace ftxui;
    // ... start TUI ...
}
```

---

## 5. Implementation Checklist

- [ ] Include `<unistd.h>` in `src/main.cpp`.
- [ ] Create the `RunHeadless` helper function in `main.cpp` to deduplicate logic.
- [ ] Add the `!isatty(STDIN_FILENO)` check at the top of `main()`.
- [ ] Read the expression from `std::cin` when in pipe mode.
- [ ] Build the project: `cmake --build build`
- [ ] Verify functionality from the terminal: 
  - `echo "5 * 5" | ./build/calc_cli` -> should output `5 * 5 = 25`
