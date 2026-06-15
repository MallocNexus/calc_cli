#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "controller/app_controller.hpp"
#include "view/app.hpp"
#include "util/formatting.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

#include <ftxui/component/screen_interactive.hpp>

static int RunHeadless(const std::string& expr) {
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

int main(int argc, char* argv[]) {
    // 1. Check for piped input (stdin is not a terminal)
    if (!isatty(STDIN_FILENO)) {
        std::string expr;
        std::getline(std::cin, expr);
        if (!expr.empty()) {
            return RunHeadless(expr);
        }
    }

    // 2. Explicit headless mode via arguments
    if (argc == 3 && std::string(argv[1]) == "--expr") {
        return RunHeadless(argv[2]);
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
