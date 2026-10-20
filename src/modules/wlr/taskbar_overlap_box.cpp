#include "modules/wlr/taskbar_overlap_box.hpp"

#include <gtkmm/button.h>

#include <algorithm>

#include "modules/wlr/taskbar_groups.hpp"

namespace waybar::modules::wlr {

OverlapBox::OverlapBox(Gtk::Orientation orientation) : orientation_{orientation} {
  set_has_window(false);
}

OverlapBox::~OverlapBox() {
  /* The children belong to someone else (task buttons): unparent, never destroy them. */
  for (auto* child : std::vector<Gtk::Widget*>(children_)) {
    child->unparent();
  }
  children_.clear();
}

void OverlapBox::pack(Gtk::Widget& widget) { add(widget); }

void OverlapBox::reorder_child(Gtk::Widget& widget, int position) {
  auto it = std::find(children_.begin(), children_.end(), &widget);
  if (it == children_.end()) {
    return;
  }
  children_.erase(it);
  position = std::clamp(position, 0, static_cast<int>(children_.size()));
  children_.insert(children_.begin() + position, &widget);
  queue_resize();
}

int OverlapBox::child_position(const Gtk::Widget& widget) const {
  auto it = std::find(children_.begin(), children_.end(), &widget);
  return it == children_.end() ? -1 : static_cast<int>(it - children_.begin());
}

GType OverlapBox::child_type_vfunc() const { return Gtk::Widget::get_type(); }

void OverlapBox::forall_vfunc(gboolean, GtkCallback callback, gpointer callback_data) {
  /* The callback may remove children: walk a copy. The order is the drawing order. */
  for (auto* child : std::vector<Gtk::Widget*>(children_)) {
    callback(child->gobj(), callback_data);
  }
}

void OverlapBox::on_add(Gtk::Widget* child) {
  children_.push_back(child);
  child->set_parent(*this);
}

void OverlapBox::on_remove(Gtk::Widget* child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it == children_.end()) {
    return;
  }
  const bool visible = child->get_visible();
  children_.erase(it);
  child->unparent();
  if (visible) {
    queue_resize();
  }
}

Gtk::SizeRequestMode OverlapBox::get_request_mode_vfunc() const {
  return Gtk::SIZE_REQUEST_CONSTANT_SIZE;
}

void OverlapBox::largest_child(bool width, int& minimum, int& natural) const {
  minimum = 0;
  natural = 0;
  for (auto* child : children_) {
    if (!child->get_visible()) {
      continue;
    }
    int child_minimum = 0;
    int child_natural = 0;
    if (width) {
      child->get_preferred_width(child_minimum, child_natural);
    } else {
      child->get_preferred_height(child_minimum, child_natural);
    }
    minimum = std::max(minimum, child_minimum);
    natural = std::max(natural, child_natural);
  }
}

void OverlapBox::get_preferred_width_vfunc(int& minimum, int& natural) const {
  largest_child(true, minimum, natural);
}

void OverlapBox::get_preferred_height_vfunc(int& minimum, int& natural) const {
  largest_child(false, minimum, natural);
}

void OverlapBox::get_preferred_width_for_height_vfunc(int, int& minimum, int& natural) const {
  largest_child(true, minimum, natural);
}

void OverlapBox::get_preferred_height_for_width_vfunc(int, int& minimum, int& natural) const {
  largest_child(false, minimum, natural);
}

void OverlapBox::on_size_allocate(Gtk::Allocation& allocation) {
  set_allocation(allocation);
  const bool horizontal = orientation_ == Gtk::ORIENTATION_HORIZONTAL;

  std::vector<Gtk::Widget*> visible;
  std::vector<int> sizes;
  for (auto* child : children_) {
    if (!child->get_visible()) {
      continue;
    }
    int minimum = 0;
    int natural = 0;
    if (horizontal) {
      child->get_preferred_width(minimum, natural);
    } else {
      child->get_preferred_height(minimum, natural);
    }
    visible.push_back(child);
    sizes.push_back(natural);
  }

  const int available = horizontal ? allocation.get_width() : allocation.get_height();
  const auto offsets = groups::overlap_offsets(available, sizes);
  for (std::size_t index = 0; index < visible.size(); ++index) {
    Gtk::Allocation child_allocation;
    if (horizontal) {
      child_allocation.set_x(allocation.get_x() + offsets[index]);
      child_allocation.set_y(allocation.get_y());
      child_allocation.set_width(sizes[index]);
      child_allocation.set_height(allocation.get_height());
    } else {
      child_allocation.set_x(allocation.get_x());
      child_allocation.set_y(allocation.get_y() + offsets[index]);
      child_allocation.set_width(allocation.get_width());
      child_allocation.set_height(sizes[index]);
    }
    visible[index]->size_allocate(child_allocation);
  }

  /* Clicks go to the topmost input window, and mapping a button (show, re-parent) raises
   * its window. Restack them in drawing order so that where buttons overlap, the one
   * drawn on top is the one that gets the click. */
  for (auto* child : visible) {
    auto* button = dynamic_cast<Gtk::Button*>(child);
    if (button && button->get_realized()) {
      if (GdkWindow* input = gtk_button_get_event_window(button->gobj())) {
        gdk_window_raise(input);
      }
    }
  }
}

}  // namespace waybar::modules::wlr
