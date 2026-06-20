#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <string>

#include "controller/app_controller.hpp"
#include "controller/exchange_rate_controller.hpp"
#include "controller/history_controller.hpp"
#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "model/exchange_rate.hpp"
#include "model/history_repository.hpp"
#include "util/constants.hpp"
#include "util/formatting.hpp"
#include "view/app.hpp"

static std::string GetDatabasePath() {
    const char* home = std::getenv(calc_cli::kFileSysEnvHome.data());
    if (!home) {
        home = std::getenv(calc_cli::kFileSysEnvUserProfile.data());
    }

    std::string db_dir;
    if (home) {
        db_dir = std::string(home) + std::string(calc_cli::kFileSysAppDataSubdir);
    } else {
        db_dir = std::string(calc_cli::kFileSysAppDataFallbackDir);
    }

    std::filesystem::create_directories(db_dir);
    return db_dir + std::string(calc_cli::kFileSysHistoryDbFilename);
}

static std::string GetExchangeRateDatabasePath() {
    const char* home = std::getenv(calc_cli::kFileSysEnvHome.data());
    if (!home) {
        home = std::getenv(calc_cli::kFileSysEnvUserProfile.data());
    }

    std::string db_dir;
    if (home) {
        db_dir = std::string(home) + std::string(calc_cli::kFileSysAppDataSubdir);
    } else {
        db_dir = std::string(calc_cli::kFileSysAppDataFallbackDir);
    }

    std::filesystem::create_directories(db_dir);
    return db_dir + std::string(calc_cli::kFileSysExchangeRateDbFilename);
}

static int RunHeadless(const std::string& expr, bool print_only_value,
                       HistoryController& history_ctrl, ExchangeRateController& exch_ctrl,
                       const std::string& exchange_arg = "") {
    std::string final_expr = expr;
    if (!exchange_arg.empty()) {
        // Parse base and quote from exchange_arg, e.g. "(AUD,USD)" or "AUD,USD"
        std::string clean = exchange_arg;
        if (clean.front() == '(' && clean.back() == ')') {
            clean = clean.substr(1, clean.size() - 2);
        }
        auto comma = clean.find(',');
        if (comma == std::string::npos || comma == 0 || comma == clean.size() - 1) {
            std::cerr << "Error: Invalid exchange format. Expected (BASE,QUOTE)\n";
            return EXIT_FAILURE;
        }
        std::string base = clean.substr(0, comma);
        std::string quote = clean.substr(comma + 1);
        final_expr = expr + " exchange(" + base + "," + quote + ")";
    }

    std::string formatted = util::FormatExpression(final_expr);
    Calculator calc;

    auto resolver = [&exch_ctrl](const std::string& base, const std::string& quote) {
        return exch_ctrl.GetRate(base, quote);
    };

    EvaluationResult res = calc.Evaluate(formatted, resolver);

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
    std::string exchange_arg = "";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--stdin-file" && i + 1 < argc) {
            std::freopen(argv[i + 1], "r", stdin);
            i++;  // skip file path
        } else if (arg == "--value") {
            print_only_value = true;
        } else if (arg == "--expr" && i + 1 < argc) {
            expr_arg = argv[i + 1];
            i++;  // skip expression value
        } else if (arg == "--exchange" && i + 1 < argc) {
            exchange_arg = argv[i + 1];
            i++;  // skip exchange format
        } else if (arg == "--help") {
            // clang-format off
            std::cout
                << "Usage: calc_cli [options]\n"
                << "Options:\n"
                << "  --expr \"EXPRESSION\"         Evaluate the given expression in headless mode\n"
                << "  --exchange \"(BASE,QUOTE)\"   Append exchange(BASE,QUOTE) to the expression\n"
                << "  --value                     Print only the result value without expression\n"
                << "  --stdin-file FILE           Read expression from the specified file instead of stdin\n"
                << "  --help                      Show this help message\n";
            // clang-format on
            return EXIT_SUCCESS;
        } else if (arg == "--version") {
            std::cout << "calc-cli version " << std::string(calc_cli::kAppVersion) << "\n";
            return EXIT_SUCCESS;
        } else if (expr_arg.empty() && arg.find("--") != 0) {
            // Support passing raw expression without --expr, e.g. calc_cli "2 + 2"
            expr_arg = arg;
        }
    }

    // Initialize Database and History Controllers
    std::string db_path = GetDatabasePath();
    HistoryRepository history_repo(db_path);
    if (!history_repo.Initialize()) {
        std::cerr << "Warning: Failed to initialize history database." << std::endl;
    }
    HistoryController history_ctrl(history_repo);

    // Initialize Exchange Database and Controller
    std::string exch_db_path = GetExchangeRateDatabasePath();
    ExchangeRate exch_repo(exch_db_path);
    if (!exch_repo.Initialize()) {
        std::cerr << "Warning: Failed to initialize exchange rate database." << std::endl;
    }
    ExchangeRateController exch_ctrl(exch_repo);

    // 1. Explicit headless mode via arguments (takes precedence)
    if (!expr_arg.empty()) {
        return RunHeadless(expr_arg, print_only_value, history_ctrl, exch_ctrl, exchange_arg);
    }

    // 2. Check for piped input (stdin is not a terminal)
    if (!isatty(STDIN_FILENO)) {
        std::string expr;
        std::getline(std::cin, expr);
        if (!expr.empty()) {
            return RunHeadless(expr, print_only_value, history_ctrl, exch_ctrl, exchange_arg);
        }
    }

    // 3. Default to interactive mode using FTXUI
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    AppState state;
    Calculator calc;
    AppController controller(state, calc, history_ctrl, exch_ctrl,
                             [&screen] { screen.ExitLoopClosure()(); });
    App app(state, controller);

    screen.Loop(app.GetComponent());

    return EXIT_SUCCESS;
}
