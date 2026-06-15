#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "model/history_repository.hpp"
#include "model/exchange_rate.hpp"
#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "controller/exchange_rate_controller.hpp"
#include "view/app.hpp"
#include "util/formatting.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <memory>

using namespace ftxui;
using Catch::Matchers::WithinRel;

// ===========================================================================
// Unit Tests — Util
// ===========================================================================

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

// ===========================================================================
// Unit Tests — Model: Calculator  (calc_lib only, zero FTXUI)
// ===========================================================================

TEST_CASE("Calculator — basic arithmetic", "[calculator]") {
    Calculator calc;

    SECTION("Addition") {
        auto res = calc.Evaluate("2 + 3");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(5.0, 1e-9));
    }

    SECTION("Subtraction") {
        auto res = calc.Evaluate("10 - 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(6.0, 1e-9));
    }

    SECTION("Multiplication") {
        auto res = calc.Evaluate("3 * 7");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(21.0, 1e-9));
    }

    SECTION("Division") {
        auto res = calc.Evaluate("15 / 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(3.75, 1e-9));
    }
}

TEST_CASE("Calculator — operator precedence", "[calculator]") {
    Calculator calc;

    SECTION("Multiplication before addition") {
        auto res = calc.Evaluate("2 + 3 * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(14.0, 1e-9));
    }

    SECTION("Parentheses override precedence") {
        auto res = calc.Evaluate("(2 + 3) * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(20.0, 1e-9));
    }

    SECTION("Nested parentheses") {
        auto res = calc.Evaluate("3 + 4 * (2 - 1)");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(7.0, 1e-9));
    }
}

TEST_CASE("Calculator — unary minus and floats", "[calculator]") {
    Calculator calc;

    SECTION("Unary minus") {
        auto res = calc.Evaluate("-3 + 1");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-2.0, 1e-9));
    }

    SECTION("Floating point input") {
        auto res = calc.Evaluate("1.5 + 2.5");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(4.0, 1e-9));
    }

    SECTION("Negative result") {
        auto res = calc.Evaluate("3 - 10");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-7.0, 1e-9));
    }
}

TEST_CASE("Calculator — error cases", "[calculator]") {
    Calculator calc;

    SECTION("Division by zero") {
        auto res = calc.Evaluate("5 / 0");
        REQUIRE_FALSE(res.ok);
        REQUIRE_FALSE(res.error.empty());
    }

    SECTION("Empty expression") {
        auto res = calc.Evaluate("");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Invalid characters") {
        auto res = calc.Evaluate("abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Trailing garbage") {
        auto res = calc.Evaluate("3 + 4 abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Mismatched parenthesis") {
        auto res = calc.Evaluate("(3 + 4");
        REQUIRE_FALSE(res.ok);
    }
}

TEST_CASE("HistoryRepository & HistoryController — persistence", "[history]") {
    HistoryRepository repo(":memory:");
    REQUIRE(repo.Initialize());
    HistoryController history_ctrl(repo);

    SECTION("History is empty initially") {
        REQUIRE(history_ctrl.GetHistory().empty());
    }

    SECTION("Successful evaluations are recorded") {
        history_ctrl.OnSaveHistory("1 + 1", "2");
        history_ctrl.OnSaveHistory("2 * 3", "6");
        REQUIRE(history_ctrl.GetHistory().size() == 2);
        REQUIRE(history_ctrl.GetHistory()[0].first == "1 + 1");
        REQUIRE(history_ctrl.GetHistory()[0].second == "2");
        REQUIRE(history_ctrl.GetHistory()[1].first == "2 * 3");
        REQUIRE(history_ctrl.GetHistory()[1].second == "6");
    }

    SECTION("ClearHistory empties the record") {
        history_ctrl.OnSaveHistory("1 + 1", "2");
        history_ctrl.OnClearHistory();
        REQUIRE(history_ctrl.GetHistory().empty());
    }
}

// ===========================================================================
// Unit Tests — Controller: AppController  (controller_lib only, zero FTXUI)
// ===========================================================================

TEST_CASE("AppController — OnEvaluate", "[controller]") {
    AppState state;
    Calculator calc;
    HistoryRepository repo(":memory:");
    repo.Initialize();
    HistoryController history_ctrl(repo);

    ExchangeRate exch_repo(":memory:");
    exch_repo.Initialize();
    ExchangeRateController exch_ctrl(exch_repo);

    AppController controller(state, calc, history_ctrl, exch_ctrl, [] {});

    SECTION("Success writes formatted result to state") {
        state.expression_input = "3 + 4";
        controller.OnEvaluate();
        REQUIRE(state.result_display == "= 7");
        REQUIRE_FALSE(state.error_state);
    }

    SECTION("Error sets error_state and prefixes message") {
        state.expression_input = "1 / 0";
        controller.OnEvaluate();
        REQUIRE(state.error_state);
        REQUIRE(state.result_display.rfind("Error:", 0) == 0);
    }

    SECTION("Successful evaluation adds to history") {
        state.expression_input = "2 * 3";
        controller.OnEvaluate();
        REQUIRE(controller.GetHistory().size() == 1);
        REQUIRE(controller.GetHistory()[0].first == "2 * 3");
    }
}

TEST_CASE("AppController — OnClear", "[controller]") {
    AppState state;
    Calculator calc;
    HistoryRepository repo(":memory:");
    repo.Initialize();
    HistoryController history_ctrl(repo);

    ExchangeRate exch_repo(":memory:");
    exch_repo.Initialize();
    ExchangeRateController exch_ctrl(exch_repo);

    AppController controller(state, calc, history_ctrl, exch_ctrl, [] {});

    state.expression_input = "some expr";
    state.result_display = "= 42";
    state.error_state = true;
    state.cursor_position = 5;

    controller.OnClear();

    REQUIRE(state.expression_input.empty());
    REQUIRE(state.result_display.empty());
    REQUIRE_FALSE(state.error_state);
    REQUIRE(state.cursor_position == 0);
}

TEST_CASE("AppController — OnQuit", "[controller]") {
    AppState state;
    Calculator calc;
    HistoryRepository repo(":memory:");
    repo.Initialize();
    HistoryController history_ctrl(repo);

    ExchangeRate exch_repo(":memory:");
    exch_repo.Initialize();
    ExchangeRateController exch_ctrl(exch_repo);

    bool quit_called = false;
    AppController controller(state, calc, history_ctrl, exch_ctrl, [&] { quit_called = true; });

    controller.OnQuit();
    REQUIRE(quit_called);
}

TEST_CASE("AppController — modal flags", "[controller]") {
    AppState state;
    Calculator calc;
    HistoryRepository repo(":memory:");
    repo.Initialize();
    HistoryController history_ctrl(repo);

    ExchangeRate exch_repo(":memory:");
    exch_repo.Initialize();
    ExchangeRateController exch_ctrl(exch_repo);

    AppController controller(state, calc, history_ctrl, exch_ctrl, [] {});

    SECTION("Version modal open/close") {
        REQUIRE_FALSE(state.show_version_modal);
        controller.OnOpenVersion();
        REQUIRE(state.show_version_modal);
        controller.OnCloseVersion();
        REQUIRE_FALSE(state.show_version_modal);
    }
}

TEST_CASE("AppController — OnClearHistory", "[controller]") {
    AppState state;
    Calculator calc;
    HistoryRepository repo(":memory:");
    repo.Initialize();
    HistoryController history_ctrl(repo);

    ExchangeRate exch_repo(":memory:");
    exch_repo.Initialize();
    ExchangeRateController exch_ctrl(exch_repo);

    AppController controller(state, calc, history_ctrl, exch_ctrl, [] {});

    history_ctrl.OnSaveHistory("1 + 1", "2");
    REQUIRE_FALSE(controller.GetHistory().empty());

    controller.OnClearHistory();
    REQUIRE(controller.GetHistory().empty());
}

// ===========================================================================
// Integration Tests — View: App  (headless FTXUI event simulation)
//
// Focus order in Container::Vertical({top_menu, tab_container, expr_input}):
//   ArrowDown 1: top_menu (horizontal, ignores ArrowDown) -> tab_container
//   ArrowDown 2: tab_container (horizontal sub-menu ignores ArrowDown) -> expr_input
// ===========================================================================

static void FocusExprInput(ftxui::Component& comp) {
    comp->OnEvent(Event::ArrowDown);  // top_menu -> tab_container
    comp->OnEvent(Event::ArrowDown);  // tab_container -> expr_input
}

// Helper to build standard test fixtures
static auto MakeApp(bool* quit_called = nullptr) {
    struct Fixture {
        AppState state;
        Calculator calc;
        std::unique_ptr<HistoryRepository> history_repo;
        std::unique_ptr<HistoryController> history_ctrl;
        std::unique_ptr<ExchangeRate> exch_repo;
        std::unique_ptr<ExchangeRateController> exch_ctrl;
        std::unique_ptr<AppController> controller;
        std::unique_ptr<App> app;
        ftxui::Component comp;
    };
    auto f = std::make_shared<Fixture>();
    f->history_repo = std::make_unique<HistoryRepository>(":memory:");
    f->history_repo->Initialize();
    f->history_ctrl = std::make_unique<HistoryController>(*f->history_repo);
    
    f->exch_repo = std::make_unique<ExchangeRate>(":memory:");
    f->exch_repo->Initialize();
    f->exch_ctrl = std::make_unique<ExchangeRateController>(*f->exch_repo);

    bool* qc = quit_called;
    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [qc] {
            if (qc) *qc = true;
        });
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();
    return f;
}

TEST_CASE("App — evaluate expression via keypress", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "3 + 4";
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.result_display == "= 7");
    REQUIRE_FALSE(f->state.error_state);
}

TEST_CASE("App — error display on bad expression", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "1 / 0";
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.error_state);
    REQUIRE(f->state.result_display.rfind("Error:", 0) == 0);
}

TEST_CASE("App — Escape clears input and result", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "42";
    f->state.result_display = "= 42";
    f->comp->OnEvent(Event::Escape);
    REQUIRE(f->state.expression_input.empty());
    REQUIRE(f->state.result_display.empty());
    REQUIRE_FALSE(f->state.error_state);
}

TEST_CASE("App — Edit -> Clear via menu", "[app]") {
    auto f = MakeApp();
    f->state.expression_input = "some expression";
    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Clear
    REQUIRE(f->state.expression_input.empty());
    REQUIRE(f->state.result_display.empty());
}

TEST_CASE("App — Edit -> Clear History via menu", "[app]") {
    auto f = MakeApp();
    f->history_ctrl->OnSaveHistory("1 + 1", "2");
    REQUIRE_FALSE(f->controller->GetHistory().empty());

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu
    f->comp->OnEvent(Event::ArrowRight);   // Clear Input -> Clear History
    f->comp->OnEvent(Event::Return);       // trigger Clear History

    REQUIRE(f->controller->GetHistory().empty());
}

TEST_CASE("App — Help -> Version opens modal", "[app]") {
    auto f = MakeApp();
    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowRight);   // Exchange -> Help
    f->comp->OnEvent(Event::ArrowDown);    // focus into Help sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Version
    REQUIRE(f->state.show_version_modal);
}

TEST_CASE("App — File -> Quit calls on_quit", "[app]") {
    bool quit_called = false;
    auto f = MakeApp(&quit_called);
    f->comp->OnEvent(Event::ArrowDown);    // focus into File sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Quit
    REQUIRE(quit_called);
}

// ===========================================================================
// Unit Tests — Model: ExchangeRate & ExchangeRateController
// ===========================================================================

TEST_CASE("ExchangeRate Cache Model", "[exchange_rate]") {
    ExchangeRate repo(":memory:");
    REQUIRE(repo.Initialize());

    SECTION("Get cached rate for missing pair returns false") {
        CachedRate rate;
        REQUIRE_FALSE(repo.GetCachedRate("AUD", "USD", rate));
    }

    SECTION("Save and retrieve rate") {
        REQUIRE(repo.SaveRate("AUD", "USD", 0.70));
        CachedRate rate;
        REQUIRE(repo.GetCachedRate("AUD", "USD", rate));
        REQUIRE(rate.rate == 0.70);
        REQUIRE(rate.timestamp > 0);
    }
}

TEST_CASE("Parser - exchange function with mock resolver", "[parser]") {
    auto mock_resolver = [](const std::string& base, const std::string& quote) {
        if (base == "AUD" && quote == "USD") return 0.70;
        throw std::runtime_error("Unsupported currency pair");
    };

    Calculator calc;
    
    SECTION("Basic exchange lookup") {
        auto res = calc.Evaluate("exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(0.70, 1e-9));
    }

    SECTION("Basic exchange lookup with implicit multiplication") {
        auto res = calc.Evaluate("100 exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(70.0, 1e-9));
    }

    SECTION("Expression-wide suffix precedence") {
        auto res = calc.Evaluate("2 + 4 + 7 * 9 exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(48.3, 1e-9)); // (2 + 4 + 63) * 0.70 = 69 * 0.70 = 48.3
    }

    SECTION("Prefix / term integration") {
        auto res = calc.Evaluate("5 + exchange(AUD, USD)", mock_resolver);
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(5.70, 1e-9));
    }
}

TEST_CASE("App — Exchange -> AUD -> USD inserts shorthand", "[app]") {
    auto f = MakeApp();
    f->state.expression_input = "100 ";
    f->state.cursor_position = 4;

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu (AUD -> USD)
    f->comp->OnEvent(Event::Return);       // trigger insert

    REQUIRE(f->state.expression_input == "100 exchange(AUD, USD)");
}

TEST_CASE("App — Exchange -> Custom opens modal and handles OK/Cancel", "[app]") {
    auto f = MakeApp();
    
    SECTION("Cancel closes custom modal without changes") {
        f->state.expression_input = "100 ";
        f->state.cursor_position = 4;

        f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
        f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
        f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu
        f->comp->OnEvent(Event::ArrowRight);   // AUD -> USD -> Custom
        f->comp->OnEvent(Event::Return);       // open custom modal

        REQUIRE(f->state.show_custom_modal);

        f->comp->OnEvent(Event::Escape);       // trigger cancel via Esc

        REQUIRE_FALSE(f->state.show_custom_modal);
        REQUIRE(f->state.expression_input == "100 ");
    }

    SECTION("OK inserts custom exchange function") {
        f->state.expression_input = "100 ";
        f->state.cursor_position = 4;

        f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
        f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
        f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu
        f->comp->OnEvent(Event::ArrowRight);   // AUD -> USD -> Custom
        f->comp->OnEvent(Event::Return);       // open custom modal

        REQUIRE(f->state.show_custom_modal);

        f->state.custom_source_input = "EUR";
        f->state.custom_target_input = "GBP";

        f->comp->OnEvent(Event::Return);       // trigger OK via Enter

        REQUIRE_FALSE(f->state.show_custom_modal);
        REQUIRE(f->state.expression_input == "100 exchange(EUR, GBP)");
    }
}
