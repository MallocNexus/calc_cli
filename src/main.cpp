#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "controller/app_controller.hpp"
#include "view/app.hpp"
#include "util/formatting.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdio>

#include <ftxui/component/screen_interactive.hpp>

static int RunHeadless(const std::string& expr, bool print_only_value) {
    std::string formatted = util::FormatExpression(expr);
    Calculator calc;
    EvaluationResult res = calc.Evaluate(formatted);
    
    if (res.ok) {
        if (print_only_value) {
            std::cout << util::FormatDouble(res.value) << "\n";
        } else {
            std::cout << formatted << " = " << util::FormatDouble(res.value) << "\n";
        }
        return EXIT_SUCCESS;
    } else {
        std::cerr << "Error: " << res.error << "\n";
        return EXIT_FAILURE;
    }
}

int main(int argc, char* argv[]) {
    bool print_only_value = false;
    std::string expr_arg = "";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--stdin-file" && i + 1 < argc) {
            std::freopen(argv[i + 1], "r", stdin);
            i++; // skip file path
        } else if (arg == "--value") {
            print_only_value = true;
        } else if (arg == "--expr" && i + 1 < argc) {
            expr_arg = argv[i + 1];
            i++; // skip expression value
        }
    }

    // 1. Explicit headless mode via arguments (takes precedence)
    if (!expr_arg.empty()) {
        return RunHeadless(expr_arg, print_only_value);
    }

    // 2. Check for piped input (stdin is not a terminal)
    if (!isatty(STDIN_FILENO)) {
        std::string expr;
        std::getline(std::cin, expr);
        if (!expr.empty()) {
            return RunHeadless(expr, print_only_value);
        }
    }
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    AppState state;
    Calculator calc;
    AppController controller(state, calc,
                             [&screen] { screen.ExitLoopClosure()(); });
    App app(state, controller);

    screen.Loop(app.GetComponent());
    return EXIT_SUCCESS;
}
