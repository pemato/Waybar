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

// Returns the icon size that lets count buttons fit side by side in available pixels,
// where each button needs overhead pixels besides its icon. The result stays within
// [min_size, max_size] (min_size is capped at max_size) and is never below 1. An empty
// box, or one not allocated yet, keeps max_size.
inline int fit_icon_size(int available, std::size_t count, int overhead, int max_size,
                         int min_size) {
  if (count == 0 || available <= 0) return max_size;
  min_size = std::min(min_size, max_size);
  const int fitting = available / static_cast<int>(count) - overhead;
  return std::max(std::clamp(fitting, min_size, max_size), 1);
}

}  // namespace waybar::modules::wlr::groups
