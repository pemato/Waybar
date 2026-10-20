#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <gtkmm.h>

#include <algorithm>

#include "modules/wlr/taskbar_overlap_box.hpp"

namespace {

// Needs a display (e.g. xvfb-run); returns false so the test can skip without one.
bool init_gtk() {
  static const bool ok = [] {
    if (!gtk_init_check(nullptr, nullptr)) return false;
    Gtk::Main::init_gtkmm_internals();
    return true;
  }();
  return ok;
}

// Runs the main loop long enough for the frame clock to lay the widgets out.
void flush() {
  bool done = false;
  g_timeout_add(
      200,
      [](gpointer data) -> gboolean {
        *static_cast<bool*>(data) = true;
        return G_SOURCE_REMOVE;
      },
      &done);
  while (!done) {
    g_main_context_iteration(nullptr, TRUE);
  }
}

// Position of a button's input window in its parent's stacking list (0 = topmost).
long stack_index(Gtk::Button& button) {
  GdkWindow* input = gtk_button_get_event_window(button.gobj());
  GList* stack = gdk_window_peek_children(gdk_window_get_parent(input));
  return g_list_index(stack, input);
}

}  // namespace

TEST_CASE("overlap box stacks button input windows in drawing order") {
  if (!init_gtk()) {
#if defined(SKIP)
    SKIP("no display");
#else
    return;
#endif
  }
  Gtk::OffscreenWindow window;
  waybar::modules::wlr::OverlapBox row(Gtk::ORIENTATION_HORIZONTAL);
  Gtk::Button first("1");
  Gtk::Button second("2");
  Gtk::Button third("3");
  row.pack(first);
  row.pack(second);
  row.pack(third);
  window.add(row);
  window.set_size_request(40, 30);  // crowded: the buttons overlap
  window.show_all();
  flush();

  // Drawing order becomes third, first, second (second on top). Re-showing a button
  // maps (and so raises) its input window; the stacking must still follow the drawing.
  row.reorder_child(third, 0);
  first.hide();
  first.show();
  flush();

  REQUIRE(stack_index(second) < stack_index(first));
  REQUIRE(stack_index(first) < stack_index(third));
}
