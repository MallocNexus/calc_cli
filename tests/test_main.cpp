#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "controller/app_controller.hpp"
#include "view/app.hpp"
#include "util/formatting.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

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

TEST_CASE("Calculator — history", "[calculator]") {
    Calculator calc;

    SECTION("History is empty initially") { REQUIRE(calc.GetHistory().empty()); }

    SECTION("Successful evaluations are recorded") {
        calc.Evaluate("1 + 1");
        calc.Evaluate("2 * 3");
        REQUIRE(calc.GetHistory().size() == 2);
        REQUIRE(calc.GetHistory()[0].first == "1 + 1");
        REQUIRE(calc.GetHistory()[1].first == "2 * 3");
    }

    SECTION("Failed evaluations are NOT recorded") {
        calc.Evaluate("bad input");
        REQUIRE(calc.GetHistory().empty());
    }

    SECTION("ClearHistory empties the record") {
        calc.Evaluate("1 + 1");
        calc.ClearHistory();
        REQUIRE(calc.GetHistory().empty());
    }
}

// ===========================================================================
// Unit Tests — Controller: AppController  (controller_lib only, zero FTXUI)
// ===========================================================================

TEST_CASE("AppController — OnEvaluate", "[controller]") {
    AppState state;
    Calculator calc;
    AppController controller(state, calc, [] {});

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
    AppController controller(state, calc, [] {});

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
    bool quit_called = false;
    AppController controller(state, calc, [&] { quit_called = true; });

    controller.OnQuit();
    REQUIRE(quit_called);
}

TEST_CASE("AppController — modal flags", "[controller]") {
    AppState state;
    Calculator calc;
    AppController controller(state, calc, [] {});

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
    AppController controller(state, calc, [] {});

    calc.Evaluate("1 + 1");
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
        std::unique_ptr<AppController> controller;
        std::unique_ptr<App> app;
        ftxui::Component comp;
    };
    auto f = std::make_shared<Fixture>();
    bool* qc = quit_called;
    f->controller = std::make_unique<AppController>(
        f->state, f->calc, [qc] {
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
    f->calc.Evaluate("1 + 1");
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
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Help
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
