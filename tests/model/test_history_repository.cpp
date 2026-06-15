#include <catch2/catch_test_macros.hpp>
#include "model/history_repository.hpp"

TEST_CASE("HistoryRepository — database operation", "[history]") {
    HistoryRepository repo(":memory:");
    REQUIRE(repo.Initialize());

    SECTION("History is empty initially") {
        REQUIRE(repo.GetHistory().empty());
    }

    SECTION("Successful evaluations are recorded") {
        REQUIRE(repo.Add("1 + 1", "2"));
        REQUIRE(repo.Add("2 * 3", "6"));
        REQUIRE(repo.GetHistory().size() == 2);
        REQUIRE(repo.GetHistory()[0].first == "1 + 1");
        REQUIRE(repo.GetHistory()[0].second == "2");
        REQUIRE(repo.GetHistory()[1].first == "2 * 3");
        REQUIRE(repo.GetHistory()[1].second == "6");
    }

    SECTION("Clear empties the record") {
        REQUIRE(repo.Add("1 + 1", "2"));
        REQUIRE(repo.Clear());
        REQUIRE(repo.GetHistory().empty());
    }
}
