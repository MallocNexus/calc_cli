#include <catch2/catch_test_macros.hpp>
#include "model/history_repository.hpp"
#include "controller/history_controller.hpp"

TEST_CASE("HistoryController — logic mapping", "[history]") {
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
