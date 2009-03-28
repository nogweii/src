/*
  This file is part of Musca.

  Musca is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as publishead by
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

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

static int (*xerrorxlib)(Display *, XErrorEvent *);

enum { WMProtocols, WMDelete, WMState, WMClass, WMName, WMLast };
enum { NetSupported, NetWMName, NetLast };
enum { MuscaCommand, MuscaResult, MuscaLast };
static Atom wmatom[WMLast], netatom[NetLast], muscatom[MuscaLast];

struct _group;
struct _head;
struct _client;

typedef struct _frame {
	int x, y, w, h;
	struct _group *group;
	struct _client *cli;
	Window win;
	ubyte flags;
	struct _frame *next;
	struct _frame *prev;
} frame;

#define FF_DEDICATE 1
#define FF_CATCHALL (1<<1)
#define ASK 0
#define SLAP 1

typedef struct _client {
	Window win;
	struct _frame *frame;
	struct _group *group;
	char name[NOTE];
	char class[NOTE];
	int x, y, w, h;
	ubyte flags;
	ubyte kids;
	// use for resize/move in stack mode
	int mxr, myr, mb, mx, my, mw, mh;
	// last known floating position
	int fx, fy, fw, fh;
	Window rl, rr, rt, rb;
	struct _client *parent;
	struct _client *next;
	struct _client *prev;
} client;

#define CF_HIDDEN 1
#define CF_KILLED (1<<1)

typedef struct _group {
	frame *frames;
	client *clients;
	int l, r, t, b;
	char name[32];
	stack *states;
	ubyte flags;
	struct _head *head;
	struct _group *next;
	struct _group *prev;
} group;

#define GF_TILING 1
#define GF_STACKING (1<<1)

typedef struct _head {
	int id;
	Screen *screen;
	group *groups;
	char *display_string;
	struct _head *next;
	struct _head *prev;
} head;

Display *display;
head *heads;

struct frame_match {
	frame *frame;
	int side;
};
#define LEFT 1
#define RIGHT 2
#define TOP 3
#define BOTTOM 4
#define FRAMES_FEWEST 1
#define FRAMES_ALL 2
#define HORIZONTAL 1
#define VERTICAL 2

struct modmask {
	char *pattern;
	ucell mask;
};
struct keymap {
	char pattern[NOTE];
	char command[NOTE];
};
struct binding {
	char pattern[NOTE];
	char command[NOTE];
	ucell mod;
	KeyCode key;
};
stack *bindings;

struct command_map {
	char *pattern;
	void (*func)(char*, regmatch_t*);
	ubyte flags;
};

stack *unmanaged;
char *self;

enum { ms_border_focus, ms_border_unfocus, ms_border_dedicate_focus, ms_border_dedicate_unfocus,
ms_border_catchall_focus, ms_border_catchall_unfocus, ms_frame_min_wh, ms_frame_resize, ms_startup,
ms_dmenu, ms_switch_window, ms_switch_group, ms_run_musca_command, ms_run_shell_command,
ms_notify, ms_stack_mouse_modifier,
ms_last };

typedef struct _setting {
	char *name;
	ubyte type;
	union {
		char *s;
		ucell u;
		dcell d;
	};
} setting;

enum { mst_str, mst_ucell, mst_dcell };
ucell NumlockMask;

typedef struct {
	Window w;
	client *c;
	frame *f;
	ubyte manage;
	Status ok;
	XWindowAttributes attr;
} winstate;

