#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include "modules/wlr/taskbar_dnd.hpp"

TEST_CASE("taskbar drag payload accepts exactly one task id") {
  constexpr uint32_t expected = 0x12345678;
  std::array<unsigned char, sizeof(expected)> payload{};
  std::memcpy(payload.data(), &expected, sizeof(expected));

  REQUIRE(waybar::modules::wlr::dnd::decode_task_id(payload.data(), payload.size()) == expected);
}

TEST_CASE("taskbar drag payload rejects malformed data") {
  std::array<unsigned char, sizeof(uint32_t) + 1> payload{};

  REQUIRE_FALSE(waybar::modules::wlr::dnd::decode_task_id(nullptr, sizeof(uint32_t)));
  REQUIRE_FALSE(waybar::modules::wlr::dnd::decode_task_id(payload.data(), 0));
  REQUIRE_FALSE(
      waybar::modules::wlr::dnd::decode_task_id(payload.data(), sizeof(uint32_t) - 1));
  REQUIRE_FALSE(waybar::modules::wlr::dnd::decode_task_id(payload.data(), payload.size()));
}

TEST_CASE("taskbar drag resolves live task positions") {
  const std::vector<uint32_t> task_ids{10, 20, 30};

  REQUIRE(waybar::modules::wlr::dnd::reorder_positions(task_ids, 10, 30) ==
          std::pair<std::size_t, std::size_t>{0, 2});
  REQUIRE(waybar::modules::wlr::dnd::reorder_positions(task_ids, 20, 20) ==
          std::pair<std::size_t, std::size_t>{1, 1});
}

TEST_CASE("taskbar drag ignores tasks that are no longer live") {
  const std::vector<uint32_t> task_ids{10, 30};

  REQUIRE_FALSE(waybar::modules::wlr::dnd::reorder_positions(task_ids, 20, 30));
  REQUIRE_FALSE(waybar::modules::wlr::dnd::reorder_positions(task_ids, 10, 20));
}

namespace dnd = waybar::modules::wlr::dnd;

TEST_CASE("test_drop_action_same_group_reorders") {
  REQUIRE(dnd::drop_action(true, false, true) == dnd::DropAction::Reorder);
  REQUIRE(dnd::drop_action(true, true, false) == dnd::DropAction::Reorder);
}

TEST_CASE("test_drop_action_other_box_ignored") {
  REQUIRE(dnd::drop_action(false, true, true) == dnd::DropAction::None);
}

TEST_CASE("test_drop_action_no_move_without_v2") {
  REQUIRE(dnd::drop_action(false, false, false) == dnd::DropAction::None);
}

TEST_CASE("test_drop_action_moves_across_groups") {
  REQUIRE(dnd::drop_action(false, false, true) == dnd::DropAction::Move);
}

TEST_CASE("test_drop_index_same_box") {
  // [A, D, B, preview, C] -> D lands between B and C.
  REQUIRE(dnd::drop_index(3, 1) == 2);
  // [A, preview, B, D, C] -> D lands between A and B.
  REQUIRE(dnd::drop_index(1, 3) == 1);
  // The preview right after D: D stays where it was.
  REQUIRE(dnd::drop_index(2, 1) == 1);
}

TEST_CASE("test_drop_index_from_other_box") {
  REQUIRE(dnd::drop_index(0, -1) == 0);
  REQUIRE(dnd::drop_index(2, -1) == 2);
}
