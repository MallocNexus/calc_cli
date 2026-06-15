#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "model/history_repository.hpp"
#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "view/app.hpp"
#include "util/formatting.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <filesystem>

#include <ftxui/component/screen_interactive.hpp>

static std::string GetDatabasePath() {
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE");
    }
    
    std::string db_dir;
    if (home) {
        db_dir = std::string(home) + "/.calc_cli";
    } else {
        db_dir = ".calc_cli";
    }

    std::filesystem::create_directories(db_dir);
    return db_dir + "/calc_history.db";
}

static int RunHeadless(const std::string& expr, bool print_only_value, HistoryController& history_ctrl) {
    std::string formatted = util::FormatExpression(expr);
    Calculator calc;
    EvaluationResult res = calc.Evaluate(formatted);
    
    if (res.ok) {
        std::string result_str = util::FormatDouble(res.value);
        if (print_only_value) {
            std::cout << result_str << "\n";
        } else {
            std::cout << formatted << " = " << result_str << "\n";
        }
        history_ctrl.OnSaveHistory(formatted, result_str);
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

    // Initialize Database and History Controllers
    std::string db_path = GetDatabasePath();
    HistoryRepository history_repo(db_path);
    if (!history_repo.Initialize()) {
        std::cerr << "Warning: Failed to initialize history database." << std::endl;
    }
    HistoryController history_ctrl(history_repo);

    // 1. Explicit headless mode via arguments (takes precedence)
    if (!expr_arg.empty()) {
        return RunHeadless(expr_arg, print_only_value, history_ctrl);
    }

    // 2. Check for piped input (stdin is not a terminal)
    if (!isatty(STDIN_FILENO)) {
        std::string expr;
        std::getline(std::cin, expr);
        if (!expr.empty()) {
            return RunHeadless(expr, print_only_value, history_ctrl);
        }
    }
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    AppState state;
    Calculator calc;
    AppController controller(state, calc, history_ctrl,
                             [&screen] { screen.ExitLoopClosure()(); });
    App app(state, controller);

    screen.Loop(app.GetComponent());
    return EXIT_SUCCESS;
}
