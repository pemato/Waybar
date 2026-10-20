#pragma once

#include <gtkmm/container.h>

#include <vector>

namespace waybar::modules::wlr {

/* A row of children that never asks for more room than its largest child. Children
 * that fit are laid out side by side; when they do not, they overlap evenly so the
 * whole row fits, later children drawn over earlier ones. */
class OverlapBox : public Gtk::Container {
 public:
  explicit OverlapBox(Gtk::Orientation orientation);
  ~OverlapBox() override;

  void pack(Gtk::Widget& widget);
  void reorder_child(Gtk::Widget& widget, int position);
  int child_position(const Gtk::Widget& widget) const;

 protected:
  GType child_type_vfunc() const override;
  void forall_vfunc(gboolean include_internals, GtkCallback callback,
                    gpointer callback_data) override;
  void on_add(Gtk::Widget* child) override;
  void on_remove(Gtk::Widget* child) override;
  Gtk::SizeRequestMode get_request_mode_vfunc() const override;
  void get_preferred_width_vfunc(int& minimum, int& natural) const override;
  void get_preferred_height_vfunc(int& minimum, int& natural) const override;
  void get_preferred_width_for_height_vfunc(int height, int& minimum,
                                            int& natural) const override;
  void get_preferred_height_for_width_vfunc(int width, int& minimum,
                                            int& natural) const override;
  void on_size_allocate(Gtk::Allocation& allocation) override;

 private:
  void largest_child(bool width, int& minimum, int& natural) const;

  Gtk::Orientation orientation_;
  std::vector<Gtk::Widget*> children_;
};

}  // namespace waybar::modules::wlr
