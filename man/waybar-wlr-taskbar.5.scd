waybar-wlr-taskbar(5)

# NAME

waybar - wlr taskbar module

# DESCRIPTION

The *taskbar* module displays the currently open applications. This module requires
a compositor that implements the foreign-toplevel-manager interface.

# CONFIGURATION

Addressed by *wlr/taskbar*

*all-outputs*: ++
	typeof: bool ++
	default: false ++
	If set to false applications on the waybar's current output will be shown. Otherwise, all applications are shown.

*bar-css-states*: ++
	typeof: bool ++
	default: false ++
	If set to true, application state is exposed as CSS classes on the Waybar
	window. Maximized and fullscreen state is aggregated across applications
	known to belong to the active workspace. See *Bar state style* below.

*format*: ++
	typeof: string ++
	default: {icon} ++
	The format, how information should be displayed.

*icon-theme*: ++
	typeof: array|string ++
	The names of the icon-themes that should be used to find an icon. The list will be traversed from left to right. If omitted, the system default will be used.

*icon-size*: ++
	typeof: integer ++
	default: 16 ++
	The size of the icon.

*markup*: ++
	typeof: bool ++
	default: false ++
	If set to true, pango markup will be accepted in format and tooltip-format.

*tooltip*: ++
	typeof: bool ++
	default: true ++
	If set to false no tooltip will be shown.

*tooltip-format*: ++
	typeof: string ++
	default: {title} ++
	The format, how information in the tooltip should be displayed.

*active-first*: ++
	typeof: bool ++
	default: false ++
	If set to true, always reorder the tasks in the taskbar so that the currently active one is first. Otherwise don't reorder.

*active-only*: ++
	typeof: bool ++
	default: false ++
	If set to true, only the currently active application button is shown.
	Other applications remain tracked and reappear when activated.

*group-by-workspace*: ++
	typeof: bool ++
	default: false ++
	If set to true, task buttons are arranged in one box per workspace, each
	headed by a clickable workspace label, followed by a trailing "other" box
	for applications whose workspace is unknown. Clicking a workspace label
	switches to that workspace, as does clicking the empty area of a box. All
	boxes get an equal share of the module's width (or height on a vertical
	bar) regardless of their contents; buttons that do not fit are clipped at
	the box edge (see *min-icon-size*). In this mode the module requests
	horizontal expansion, but the bar only gives it extra space when one of the
	bar's *expand-left*, *expand-center* or *expand-right* options is set for
	its section; on a vertical bar the same applies to the height. Clicking the
	"other" label does nothing. Combining this
	option with *expand* is not specifically supported; buttons may stretch
	within their boxes. Requires *ext-workspace-v1* for workspace boxes. See
	*Workspace membership* below.

*other-apps-label*: ++
	typeof: string ++
	default: other ++
	The label of the trailing box holding applications whose workspace is
	unknown. Only used with *group-by-workspace*.

*show-workspace-labels*: ++
	typeof: bool ++
	default: true ++
	With *group-by-workspace*, whether each box starts with its workspace
	name. Clicking the empty area of a box switches to that workspace, so the
	label can be hidden without losing the switcher. Ignored otherwise.

*sort-by-app-id*: ++
	typeof: bool ++
	default: false ++
	If set to true, group tasks by their app_id. Cannot be used with 'active-first'.

*homogeneous*: ++
	typeof: bool ++
	default: false ++
	If set to true, distribute every task button evenly across the taskbar's allocated width. Buttons will automatically resize so that 'N' visible tasks each take '1/N' of the width.

*justify*: ++
	typeof: string ++
	The alignment of the text within the module's box, allowing options 'left', 'right', or 'center' to define the positioning.

*expand*: ++
	typeof: bool ++
	default: false ++
	If set to true, task buttons stretch to fill the available space in the taskbar and long titles are ellipsized to fit. Only takes effect on a horizontal bar; on a vertical bar the buttons keep their content-based size. If set to false, buttons are sized to their content.

*truncate*: ++
	typeof: bool ++
	default: false ++
	If set to true, the task button text will be ellipsized (truncated with …) when the available button width is smaller than the label text.

*on-click*: ++
	typeof: string ++
	The action which should be triggered when clicking on the application button with the left mouse button.

*on-click-middle*: ++
	typeof: string ++
	The action which should be triggered when clicking on the application button with the middle mouse button.

*on-click-right*: ++
	typeof: string ++
	The action which should be triggered when clicking on the application button with the right mouse button.

*on-update*: ++
	typeof: string ++
	Command to execute when the module is updated.

*ignore-list*: ++
	typeof: array ++
	List of app_id/titles to be invisible.

*squash-list*: ++
	typeof: array ++
	List of app_id/titles whose multiple instances are collapsed into a single button. When more than one instance of a listed app is open, only one button is shown; when one instance closes, the next hidden instance reappears. The special value '\*' matches all applications.

*app_ids-mapping*: ++
	typeof: object ++
	Dictionary of app_id to be replaced with

*rewrite*: ++
	typeof: object ++
	Rules to rewrite the module format output. See *rewrite rules*.

# FORMAT REPLACEMENTS

*{icon}*: The icon of the application.

*{name}*: The application name as in desktop file if appropriate desktop files are found, otherwise same as {app_id}

*{title}*: The title of the application.

*{app_id}*: The app_id (== application name) of the application.

*{state}*: The state (minimized, maximized, active, fullscreen) of the application.

*{short_state}*: The state (minimize == m, maximized == M, active == A, fullscreen == F) represented as one character of the application.

As with all format replacements in Waybar, a length limit such as *{title:.15}* is
measured in bytes, not characters. This may produce invalid text if a multi-byte
character is split, or (when *markup* is true) if the truncation cuts through a
value that must be escaped in XML.

# CLICK ACTIONS

*activate*: Bring the application into foreground.

*minimize*: Toggle application's minimized state.

*minimize-raise*: Bring the application into foreground or toggle its minimized state.

*maximize*: Toggle application's maximized state.

*fullscreen*: Toggle application's fullscreen state.

*close*: Close the application.

# REWRITE RULES

*rewrite* is an object where keys are regular expressions and values are
rewrite rules if the expression matches. Rules may contain references to
captures of the expression.

Regular expression and replacement follow ECMA-script rules.

An expression must match the format output *fully* to trigger its replacement.

If no expression matches, the format output is left unchanged.

Invalid expressions (e.g., mismatched parentheses) are skipped.

# EXAMPLES

```
"wlr/taskbar": {
	"format": "{icon}",
	"icon-size": 14,
	"icon-theme": "Numix-Circle",
	"tooltip-format": "{title}",
	"on-click": "activate",
	"on-click-middle": "close",
	"ignore-list": [
	    "Alacritty"
	],
	"app_ids-mapping": {
		"firefoxdeveloperedition": "firefox-developer-edition"
	},
	"rewrite": {
		"Firefox Web Browser": "Firefox",
		"Foot Server": "Terminal"
	}
}
```

# Style

- *#taskbar*
- *#taskbar.empty*
- *#taskbar button*
- *#taskbar button.maximized*
- *#taskbar button.minimized*
- *#taskbar button.active*
- *#taskbar button.fullscreen*

With *group-by-workspace*:

- *#taskbar .workspace*: the box of one workspace
- *#taskbar .workspace.active*: the box of the active workspace
- *#taskbar .workspace.empty*: a box with no task buttons. The trailing
  "other" box is also marked *empty* when it has no buttons. A box whose only
  buttons are hidden by *active-only* is not marked *empty*.
- *#taskbar .workspace.other*: the trailing box of applications with unknown workspace
- *#taskbar .workspace-label*: the clickable workspace label
- *#taskbar .workspace-tasks*: the row of task buttons inside a box

# Workspace membership

Which workspace an application belongs to is determined as follows. On labwc
the membership is exact, reported through the *lab-toplevel-workspace-v1*
protocol. On other compositors it is learned when an application is activated
(this requires *ext-workspace-v1*). Applications whose workspace is not known
appear in the "other" box.

# Bar state style

When *bar-css-states* is enabled, the following classes are added to
*window#waybar*:

- *window#waybar.toplevel-active*
- *window#waybar.toplevel-maximized*
- *window#waybar.toplevel-minimized*
- *window#waybar.toplevel-fullscreen*

The active, minimized classes describe the active application. The maximized
and fullscreen classes are set if any non-minimized application known to belong
to the active workspace has that state.

Workspace membership is exact on labwc (see *Workspace membership* above). On
other compositors it is learned when an application is activated and requires
*ext-workspace-v1*, so before an application has been activated during the
current Waybar session its workspace may be unknown. On compositors without
workspace information, these classes fall back to the active application's
state.

For example:

```
window#waybar {
	background-color: rgba(0, 0, 0, 0.5);
}

window#waybar.toplevel-maximized {
	background-color: rgba(0, 0, 0, 1);
}
```
