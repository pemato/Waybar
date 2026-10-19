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
