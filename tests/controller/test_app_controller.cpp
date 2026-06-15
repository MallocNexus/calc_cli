#include <catch2/catch_test_macros.hpp>
#include "model/app_state.hpp"
#include "model/calculator.hpp"
#include "model/history_repository.hpp"
#include "model/exchange_rate.hpp"
#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "controller/exchange_rate_controller.hpp"

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
