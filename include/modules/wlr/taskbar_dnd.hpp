#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

namespace waybar::modules::wlr::dnd {

inline std::optional<uint32_t> decode_task_id(const unsigned char* data, std::size_t length) {
  if (data == nullptr || length != sizeof(uint32_t)) {
    return std::nullopt;
  }

  uint32_t id;
  std::memcpy(&id, data, sizeof(id));
  return id;
}

inline std::optional<std::pair<std::size_t, std::size_t>> reorder_positions(
    const std::vector<uint32_t>& task_ids, uint32_t dragged_id, uint32_t destination_id) {
  std::optional<std::size_t> dragged_position;
  std::optional<std::size_t> destination_position;

  for (std::size_t position = 0; position < task_ids.size(); ++position) {
    if (task_ids[position] == dragged_id) dragged_position = position;
    if (task_ids[position] == destination_id) destination_position = position;
  }

  if (!dragged_position || !destination_position) return std::nullopt;
  return std::pair{*dragged_position, *destination_position};
}

enum class DropAction { None, Reorder, Move };

// What dropping a task button does: within its own box it reorders; onto another
// workspace's box it moves the window there, if the compositor supports that; onto the
// "other" box it does nothing.
inline DropAction drop_action(bool same_group, bool destination_is_other, bool can_move) {
  if (same_group) return DropAction::Reorder;
  if (destination_is_other) return DropAction::None;
  return can_move ? DropAction::Move : DropAction::None;
}

// Where the dragged button goes once the drag preview at preview_index is removed from a
// box, given the button's own index in that box (or -1 if it comes from another box).
inline int drop_index(int preview_index, int dragged_index) {
  return dragged_index >= 0 && dragged_index < preview_index ? preview_index - 1 : preview_index;
}

}  // namespace waybar::modules::wlr::dnd
