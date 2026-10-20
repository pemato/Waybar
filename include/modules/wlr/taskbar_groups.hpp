#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace waybar::modules::wlr::groups {

// Returns the position of task_workspace_id in ordered_workspace_ids, or
// ordered_workspace_ids.size() when the task belongs to no listed workspace ("other").
inline std::size_t group_index(const std::vector<std::string>& ordered_workspace_ids,
                               const std::string& task_workspace_id) {
  for (std::size_t index = 0; index < ordered_workspace_ids.size(); ++index) {
    if (ordered_workspace_ids[index] == task_workspace_id) return index;
  }
  return ordered_workspace_ids.size();
}

// Returns a permutation of workspace indices giving the display order. Without any
// coordinates, announcement order is kept. Otherwise the indices are stable-sorted by
// their coordinates, compared lexicographically, with an empty vector sorting first.
inline std::vector<std::size_t> workspace_order(
    const std::vector<std::vector<uint32_t>>& coordinates) {
  std::vector<std::size_t> order(coordinates.size());
  std::iota(order.begin(), order.end(), std::size_t{0});

  const bool has_coordinates = std::any_of(coordinates.begin(), coordinates.end(),
                                           [](const auto& entry) { return !entry.empty(); });
  if (!has_coordinates) return order;

  std::stable_sort(order.begin(), order.end(), [&coordinates](std::size_t lhs, std::size_t rhs) {
    return coordinates[lhs] < coordinates[rhs];
  });
  return order;
}

// Returns where each of a row of children starts along the row, given their sizes and
// the space available. Children that fit are placed side by side; otherwise they overlap
// with an even step so that the last child ends exactly at the end of the space (or all
// start at 0 when even one child does not fit).
inline std::vector<int> overlap_offsets(int available, const std::vector<int>& sizes) {
  std::vector<int> offsets;
  offsets.reserve(sizes.size());
  const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
  if (total <= available) {
    int offset = 0;
    for (int size : sizes) {
      offsets.push_back(offset);
      offset += size;
    }
    return offsets;
  }
  const int count = static_cast<int>(sizes.size());
  if (count == 1) return {0};
  const int span = std::max(available - sizes.back(), 0);
  for (int index = 0; index < count; ++index) {
    offsets.push_back(index * span / (count - 1));
  }
  return offsets;
}

}  // namespace waybar::modules::wlr::groups
