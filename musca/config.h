/*
  This file is part of Musca.

  Musca is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Musca is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Musca.  If not, see <http://www.gnu.org/licenses/>.

  Sean Pringle
  sean dot pringle at gmail dot com
  https://launchpad.net/musca
*/

// general settings.
// field 1 is a name usable with the 'set' command.
// field 2 is mst_str=char, mst_ucell=unsigned int, mst_dcell = double
// field 3 is set according to field 2.  mst_str = .s, mst_ucell = .u, mst_dcell = .d
setting settings[] = {
	// these colours must be strings that X11 will recognize
	{ "border_focus",            mst_str,   { .s = "Blue"       } },
	{ "border_unfocus",          mst_str,   { .s = "Dim Gray"   } },
	{ "border_dedicate_focus",   mst_str,   { .s = "Red"        } },
	{ "border_dedicate_unfocus", mst_str,   { .s = "Dark Red"   } },
	{ "border_catchall_focus",   mst_str,   { .s = "Green"      } },
	{ "border_catchall_unfocus", mst_str,   { .s = "Dark Green" } },
	// frame size limits in pixels
	{ "frame_min_wh",            mst_ucell, { .u = 100 } },
	{ "frame_resize",            mst_ucell, { .u = 20  } },
	// optional startup file of musca commands, one per line
	{ "startup",                 mst_str,   { .s = ".musca_start" } },
	// customize the dmenu command.
	{ "dmenu",                   mst_str,   { .s = "dmenu -i -b"  } },
	// customize the actions of dmenu driven window/group/command menus.  by default we
	// just spit commands back to musca, but you can wrap or redirect stuff.  the $MUSCA
	// environment variable is set to argv[0] in setup().
	{ "switch_window",           mst_str,   { .s = "xargs -I name $MUSCA -c \"raise name\""      } },
	{ "switch_group",            mst_str,   { .s = "xargs -I name $MUSCA -c \"use name\""        } },
	{ "run_musca_command",       mst_str,   { .s = "xargs -I command $MUSCA -c \"command\""      } },
	{ "run_shell_command",       mst_str,   { .s = "xargs -I command $MUSCA -c \"exec command\"" } },
	// your preferred method of being send a message, eg:
	// print to stdout "echo %s"
	// popup xmessage "xmessage -timeout 3 %s"
	// popup dzen2 "echo \"%s\" | dzen2 -p 3 -w 300"
	// popup notify-send "notify-send -t 3000 Musca \"%s\""
	{ "notify",                  mst_str,   { .s = "echo %s" } },
	// this keyboard modifier is used in stacking mode, along with mouse buttons 1 and 3, to
	// move and resize windows respectively.
	{ "stack_mouse_modifier",    mst_str,   { .s = "Mod4"    } }
};

// default list of window *classes* to ignore.  use xprop WM_CLASS to find them.  either
// make additions here, or use the 'manage' command in your startup file.
char *unmanaged_windows[] = { "trayer", "Xmessage", "Conky" };

// when binding keys, these are the (case insensitive) modifier names we will recognize.  note
// that LockMask is explicitly ignored in grab_stuff(), so adding it here will have precisely
// zero effect :)
struct modmask modmasks[] = {
	{ "Mod1", Mod1Mask },
	{ "Mod2", Mod2Mask },
	{ "Mod3", Mod3Mask },
	{ "Mod4", Mod4Mask },
	{ "Mod5", Mod5Mask },
	{ "Control", ControlMask },
	{ "Shift",   ShiftMask   },
};

// we simply map key bindings to any musca command.  either add custom stuff here, or use
// the 'bind' command in your startup file.
struct keymap keymaps[] = {
	{ "Mod4+h",             "hsplit 1/2"    },
	{ "Mod4+v",             "vsplit 1/2"    },
	{ "Mod4+r",             "remove"        },
	{ "Mod4+o",             "only"          },
	{ "Mod4+k",             "kill"          },
	{ "Mod4+c",             "cycle"         },
	{ "Mod4+Left",          "focus left"    },
	{ "Mod4+Right",         "focus right"   },
	{ "Mod4+Up",            "focus up"      },
	{ "Mod4+Down",          "focus down"    },
	{ "Mod4+Next",          "use (next)"    },
	{ "Mod4+Prior",         "use (prev)"    },
	{ "Mod4+Tab",           "screen (next)" },
	{ "Mod4+w",             "switch window" },
	{ "Mod4+g",             "switch group"  },
	{ "Mod4+x",             "shell"         },
	{ "Mod4+m",             "command"       },
	{ "Mod4+d",             "dedicate flip" },
	{ "Mod4+a",             "catchall flip" },
	{ "Mod4+u",             "undo"          },
	{ "Mod4+s",             "stack flip"    },
	{ "Mod4+Control+Left",  "resize left"   },
	{ "Mod4+Control+Right", "resize right"  },
	{ "Mod4+Control+Up",    "resize up"     },
	{ "Mod4+Control+Down",  "resize down"   },
	{ "Mod4+t",             "exec urxvt"    },
};

// dmenu musca command options.  these are just convenient prompts for the real commands
// below in command_callbacks.
char *commands[] = {
	"hsplit", "vsplit", "width", "height", "resize", "remove", "kill", "cycle", "only",
	"focus", "lfocus", "rfocus", "ufocus", "dfocus", "dedicate", "catchall",
	"undo", "pad", "add", "drop", "name", "dump", "load", "use", "stack",
	"exec", "move", "manage", "swap", "lswap", "rswap", "uswap", "dswap",
	"raise", "switch", "set", "bind", "quit" };

// be careful if you modify these.  each callback function expects the correct number
// of matched sub-strings.
struct command_map command_callbacks[] = {
	// frames
	{ "^(hsplit|vsplit)[[:space:]]+([0-9]+%?)$",
		com_frame_split,     GF_TILING             },
	{ "^(hsplit|vsplit)[[:space:]]+([0-9]+)/([0-9]+)$",
		com_frame_split,     GF_TILING             },
	{ "^(width|height)[[:space:]]+([0-9]+%?)$",
		com_frame_size,      GF_TILING             },
	{ "^(width|height)[[:space:]]+([0-9]+)/([0-9]+))$",
		com_frame_size,      GF_TILING             },
	{ "^remove$",
		com_frame_remove,    GF_TILING             },
	{ "^kill$",
		com_frame_kill,      GF_TILING|GF_STACKING },
	{ "^cycle$",
		com_frame_cycle,     GF_TILING|GF_STACKING },
	{ "^only$",
		com_frame_only,      GF_TILING             },
	{ "^focus[[:space:]]+(left|right|up|down)$",
		com_frame_focus,     GF_TILING             },
	{ "^(lfocus|rfocus|ufocus|dfocus)$",
		com_frame_focus,     GF_TILING             },
	{ "^dedicate[[:space:]]+(on|off|flip)$",
		com_frame_dedicate,  GF_TILING             },
	{ "^catchall[[:space:]]+(on|off|flip)$",
		com_frame_catchall,  GF_TILING             },
	{ "^undo$",
		com_group_undo,      GF_TILING             },
	{ "^resize[[:space:]]+(.+)$",
		com_frame_resize,    GF_TILING             },

	// groups
	{ "^pad[[:space:]]+([0-9]+)[[:space:]]+([0-9]+)[[:space:]]+([0-9]+)[[:space:]]+([0-9]+)$",
		com_group_pad,       GF_TILING|GF_STACKING },
	{ "^add[[:space:]]+([^[:space:]]+)$",
		com_group_add,       GF_TILING|GF_STACKING },
	{ "^drop[[:space:]]+([^[:space:]]+)$",
		com_group_drop,      GF_TILING|GF_STACKING },
	{ "^name[[:space:]]+([^[:space:]]+)$",
		com_group_name,      GF_TILING|GF_STACKING },
	{ "^dump[[:space:]]+([^[:space:]]+)$",
		com_group_dump,      GF_TILING             },
	{ "^load[[:space:]]+([^[:space:]]+)$",
		com_group_load,      GF_TILING             },
	{ "^use[[:space:]]+([^[:space:]]+)$",
		com_group_use,       GF_TILING|GF_STACKING },
	{ "^stack[[:space:]]+(on|off|flip)$",
		com_group_stack,     GF_TILING|GF_STACKING },

	// windows
	{ "^(uswap|dswap|lswap|rswap)$",
		com_frame_swap,      GF_TILING             },
	{ "^swap[[:space:]]+(up|down|left|right)$",
		com_frame_swap,      GF_TILING             },
	{ "^move[[:space:]]+([^[:space:]]+)$",
		com_window_to_group, GF_TILING|GF_STACKING },
	{ "^manage[[:space:]]+(on|off)[[:space:]]+([^[:space:]]+)$",
		com_manage,          GF_TILING|GF_STACKING },
	{ "^raise[[:space:]]+(.+)$",
		com_window_raise,    GF_TILING|GF_STACKING },

	// misc
	{ "^screen[[:space:]]+(.+)$",
		com_screen_switch,   GF_TILING|GF_STACKING },
	{ "^exec[[:space:]]+(.+)$",
		com_exec,            GF_TILING|GF_STACKING },
	{ "^set[[:space:]]+([a-z0-9_]+)[[:space:]]+(.+)$",
		com_set,             GF_TILING|GF_STACKING },
	{ "^bind[[:space:]]+(on|off)[[:space:]]+([a-z0-9_+]+)(.*?)$",
		com_bind,            GF_TILING|GF_STACKING },
	{ "switch[[:space:]]+(window|group)$",
		com_switch,          GF_TILING|GF_STACKING },
	{ "command$",
		com_command,         GF_TILING|GF_STACKING },
	{ "shell$",
		com_shell,           GF_TILING|GF_STACKING },
	{ "^quit$",
		com_quit,            GF_TILING|GF_STACKING },
};
