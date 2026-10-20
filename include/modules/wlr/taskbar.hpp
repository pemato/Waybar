#pragma once

#include <gdk/gdk.h>
#include <glibmm/refptr.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <wayland-client.h>

#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "AModule.hpp"
#include "bar.hpp"
#include "client.hpp"
#include "ext-workspace-v1-client-protocol.h"
#include "giomm/desktopappinfo.h"
#include "lab-toplevel-workspace-v1-client-protocol.h"
#include "modules/wlr/taskbar_overlap_box.hpp"
#include "util/icon_loader.hpp"
#include "util/json.hpp"
#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

namespace waybar::modules::wlr {

struct widget_geometry {
  int x, y, w, h;
};

class Taskbar;

class Task {
 public:
  Task(const waybar::Bar&, const Json::Value&, Taskbar*, struct zwlr_foreign_toplevel_handle_v1*,
       struct wl_seat*);
  ~Task();

 public:
  enum State {
    MAXIMIZED = (1 << 0),
    MINIMIZED = (1 << 1),
    ACTIVE = (1 << 2),
    FULLSCREEN = (1 << 3),
    INVALID = (1 << 4)
  };
  // made public so TaskBar can reorder based on configuration.
  Gtk::Button button;
  struct widget_geometry minimize_hint;

 private:
  static uint32_t global_id;

 private:
  const waybar::Bar& bar_;
  const Json::Value& config_;
  Taskbar* tbar_;
  struct zwlr_foreign_toplevel_handle_v1* handle_;
  struct wl_seat* seat_;

  uint32_t id_;

  Gtk::Box content_;
  Gtk::Image icon_;
  Gtk::Label text_before_;
  Gtk::Label text_after_;
  Glib::RefPtr<Gio::DesktopAppInfo> app_info_;
  bool button_visible_ = false;
  bool ignored_ = false;
  bool squashed_ = false;
  /* Whether the toplevel is on this bar's output, per the protocol's
   * output_enter/output_leave events */
  bool on_bar_output_ = false;
  bool size_allocate_connected_ = false;

  bool with_icon_ = false;
  int icon_size_ = 16;
  bool with_name_ = false;
  std::string format_before_;
  std::string format_after_;

  std::string format_tooltip_;

  std::string name_;
  std::string title_;
  std::string app_id_;
  uint32_t state_ = 0;
  struct ext_workspace_handle_v1* workspace_ = nullptr;
  struct lab_toplevel_workspace_v1* workspace_obj_ = nullptr;
  std::string workspace_id_;
  sigc::connection leave_place_;  // re-places the button after a leave without an enter

  int32_t drag_start_x;
  int32_t drag_start_y;
  int32_t drag_start_button = -1;

 private:
  std::string repr() const;
  std::string state_string(bool = false) const;
  void set_minimize_hint();
  void on_button_size_allocated(Gtk::Allocation& alloc);
  void hide_if_ignored();
  void hide_if_duplicate();
  void show_button();
  void hide_button();

 public:
  /* Getter functions */
  uint32_t id() const { return id_; }
  std::string title() const { return title_; }
  std::string app_id() const { return app_id_; }
  uint32_t state() const { return state_; }
  bool maximized() const { return state_ & MAXIMIZED; }
  bool minimized() const { return state_ & MINIMIZED; }
  bool active() const { return state_ & ACTIVE; }
  bool fullscreen() const { return state_ & FULLSCREEN; }
  bool visible() const { return button_visible_; }
  struct ext_workspace_handle_v1* workspace() const { return workspace_; }
  void set_workspace(struct ext_workspace_handle_v1* workspace) { workspace_ = workspace; }
  const std::string& workspace_id() const { return workspace_id_; }
  void bind_workspace_object();
  void move_to_workspace(const std::string& workspace_id);
  void load_icon_into(Gtk::Image& image) const;
  void handle_workspace_enter(const char* id);
  void handle_workspace_leave(const char* id);

 public:
  /* Callbacks for the wlr protocol */
  void handle_title(const char*);
  void handle_app_id(const char*);
  void handle_output_enter(struct wl_output*);
  void handle_output_leave(struct wl_output*);
  void handle_state(struct wl_array*);
  void handle_done();
  void handle_closed();

  /* Callbacks for Gtk events */
  bool handle_clicked(GdkEventButton*);
  bool handle_button_release(GdkEventButton*);
  bool handle_motion_notify(GdkEventMotion*);
  void handle_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context,
                            Gtk::SelectionData& selection_data, guint info, guint time);
  void handle_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                 Gtk::SelectionData selection_data, guint info, guint time);

 public:
  bool operator==(const Task&) const;
  bool operator!=(const Task&) const;

 public:
  void update();

 public:
  /* Interaction with the tasks */
  void maximize(bool);
  void minimize(bool);
  void activate();
  void fullscreen(bool);
  void close();
};

using TaskPtr = std::unique_ptr<Task>;

class Taskbar : public waybar::AModule {
 public:
  struct WorkspaceState;

  /* One box of the "group-by-workspace" view: an optional label followed by the task
   * buttons. The buttons sit in an OverlapBox, which never asks for more room than one
   * button, so all groups keep an equal share of the module whatever they hold. The
   * buttons are owned by their Task, never by the group. */
  struct Group {
    Group(Gtk::Orientation orientation, WorkspaceState* workspace, bool show_label);
    Gtk::EventBox frame;        // hit area for clicks and drops
    Gtk::Box box;               // .workspace
    Gtk::Button label;          // .workspace-label, packed only when labels are shown
    OverlapBox tasks;           // .workspace-tasks, holds the task buttons
    WorkspaceState* workspace;  // nullptr for the "other" group
  };

  struct WorkspaceState {
    Taskbar* taskbar;
    struct ext_workspace_handle_v1* handle;
    uint32_t state = 0;
    std::string id;
    std::string name;
    std::vector<uint32_t> coordinates;
    std::unique_ptr<Group> group;
  };

  Taskbar(const std::string&, const waybar::Bar&, const Json::Value&);
  ~Taskbar();
  void update();

 private:
  const waybar::Bar& bar_;
  Gtk::Box box_;
  std::vector<TaskPtr> tasks_;

  IconLoader icon_loader_;
  std::unordered_set<std::string> ignore_list_;
  std::unordered_set<std::string> squash_list_;
  std::map<std::string, std::string> app_ids_replace_map_;

  struct zwlr_foreign_toplevel_manager_v1* manager_;
  struct ext_workspace_manager_v1* workspace_manager_;
  struct lab_toplevel_workspace_manager_v1* toplevel_workspace_manager_ = nullptr;
  uint32_t toplevel_workspace_version_ = 0;
  struct wl_seat* seat_;
  std::vector<struct ext_workspace_group_handle_v1*> workspace_groups_;
  std::vector<std::unique_ptr<WorkspaceState>> workspaces_;
  struct ext_workspace_handle_v1* current_workspace_ = nullptr;

  bool group_by_workspace_ = false;
  bool show_labels_ = true;
  bool hide_empty_other_ = false;
  std::string other_label_;
  std::unique_ptr<Group> other_group_;

  /* Drag preview: while a button is dragged over a box it can be dropped on, it is hidden
   * and drag_preview_ (showing its icon) is shown where the drop would put it. */
  std::optional<uint32_t> dragging_id_;
  std::optional<uint32_t> preview_task_id_;
  Group* preview_group_ = nullptr;
  Gtk::Widget* hovered_widget_ = nullptr;  // drop target the drag is over
  Gtk::Image drag_preview_icon_;
  Gtk::Button drag_preview_;
  sigc::connection resync_;  // re-places buttons from labwc's answer after a drop
  sigc::connection leave_idle_;  // ends the preview once the drag left the taskbar
  bool drop_pending_ = false;    // a drop happened; its data has not arrived yet

 public:
  /* Callbacks for global registration */
  void register_manager(struct wl_registry*, uint32_t name, uint32_t version);
  void register_workspace_manager(struct wl_registry*, uint32_t name, uint32_t version);
  void register_toplevel_workspace_manager(struct wl_registry*, uint32_t name, uint32_t version);
  void register_seat(struct wl_registry*, uint32_t name, uint32_t version);

  /* Callbacks for the wlr protocol */
  void handle_toplevel_create(struct zwlr_foreign_toplevel_handle_v1*);
  void handle_finished();
  void handle_workspace_group_create(struct ext_workspace_group_handle_v1*);
  void handle_workspace_group_removed(struct ext_workspace_group_handle_v1*);
  void handle_workspace_create(struct ext_workspace_handle_v1*);
  void handle_workspace_done();
  void handle_workspace_finished();
  void handle_workspace_removed(struct ext_workspace_handle_v1*);

 public:
  void add_button(Task&);
  void move_button(Task&, int);
  bool reorder_task(uint32_t dragged_id, uint32_t destination_id);
  void remove_button(Task&);
  Gtk::Container& container_for(const Task&);
  Group* group_of(const Task&);
  void activate_workspace(WorkspaceState&);
  void connect_group(Group&);
  bool handle_drop(uint32_t dragged_id, Group& destination,
                   std::optional<uint32_t> destination_task_id);
  void drag_started(Task&);
  void drag_finished(Task&);
  void drag_hover(Group&, Gtk::Widget& target, Task* hovered_task = nullptr);
  void drag_left(Gtk::Widget& target);
  void drop_started() { drop_pending_ = true; }
  void place_task(Task&);
  void rebuild_groups();
  void remove_task(uint32_t);
  void assign_current_workspace(Task&);
  void update_bar_css_classes();
  struct lab_toplevel_workspace_manager_v1* toplevel_workspace_manager() const {
    return toplevel_workspace_manager_;
  }
  bool has_exact_workspaces() const { return toplevel_workspace_manager_ != nullptr; }
  struct ext_workspace_handle_v1* handle_for_id(const std::string& id) const;

  bool show_output(struct wl_output*) const;
  bool all_outputs() const;

  const IconLoader& icon_loader() const;
  const std::unordered_set<std::string>& ignore_list() const;
  const std::unordered_set<std::string>& squash_list() const;
  const std::map<std::string, std::string>& app_ids_replace_map() const;
  std::size_t task_id_count(std::string_view id) const;
  std::size_t task_title_count(std::string_view title) const;

  auto tasks() {
    return tasks_ | std::views::transform([](auto& task) -> Task& { return *task; });
  }

 private:
  void set_bar_css_class(const std::string&, bool);
  void pack_button(Task&);
  void refresh_empty_classes();
  std::vector<WorkspaceState*> ordered_grouped_workspaces() const;
  Task* find_task(uint32_t id);
  bool can_move_to(const Group&) const;
  void show_preview(Task& dragged, Group& group, Task* hovered_task);
  void hide_preview();
};

} /* namespace waybar::modules::wlr */
