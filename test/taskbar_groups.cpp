#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "modules/wlr/taskbar_groups.hpp"

namespace groups = waybar::modules::wlr::groups;

TEST_CASE("group_index maps a known id to its position") {
  REQUIRE(groups::group_index({"1", "2", "3"}, "2") == 1);
}

TEST_CASE("test_group_index_unknown_id_goes_to_other") {
  REQUIRE(groups::group_index({"1", "2", "3"}, "nope") == 3);
  REQUIRE(groups::group_index({"1", "2", "3"}, "") == 3);
}

TEST_CASE("test_group_index_no_workspaces") {
  REQUIRE(groups::group_index({}, "1") == 0);
  REQUIRE(groups::group_index({}, "") == 0);
}

TEST_CASE("workspace_order keeps announcement order without coordinates") {
  REQUIRE(groups::workspace_order({{}, {}, {}}) == std::vector<std::size_t>{0, 1, 2});
}

TEST_CASE("workspace_order sorts by coordinates when present") {
  REQUIRE(groups::workspace_order({{2}, {0}, {1}}) == std::vector<std::size_t>{1, 2, 0});
  REQUIRE(groups::workspace_order({{1, 1}, {1, 0}, {0, 5}}) == std::vector<std::size_t>{2, 1, 0});
}

TEST_CASE("fit_icon_size keeps the maximum with room to spare") {
  REQUIRE(groups::fit_icon_size(400, 3, 8, 24, 10) == 24);
  REQUIRE(groups::fit_icon_size(400, 0, 8, 24, 10) == 24);
  REQUIRE(groups::fit_icon_size(0, 3, 8, 24, 10) == 24);
}

TEST_CASE("fit_icon_size shrinks a crowded box down to the minimum") {
  REQUIRE(groups::fit_icon_size(100, 5, 4, 24, 10) == 16);  // 100/5 - 4
  REQUIRE(groups::fit_icon_size(100, 20, 4, 24, 10) == 10);  // floor
  REQUIRE(groups::fit_icon_size(10, 20, 4, 24, 0) == 1);     // never below 1
  REQUIRE(groups::fit_icon_size(100, 20, 4, 24, 30) == 24);  // min above max
}
