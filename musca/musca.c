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

#define DEBUG 0

#include "tools.c"
#include "musca.h"
#include "musca_proto.h"
#include "config.h"

// will override this as some sort of gui notice
void say(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	char pad[BLOCK], cmd[BLOCK];
	vsnprintf(pad, BLOCK, fmt, ap);
	va_end(ap);
	snprintf(cmd, BLOCK, settings[ms_notify].s, pad);
	exec_cmd(cmd);
}
// x11 color
ucell get_color(head *h, const char *name)
{
	XColor color;
	Colormap map = DefaultColormap(display, h->id);
	return XAllocNamedColor(display, map, name, &color, &color) ? color.pixel: None;
}
// error handler courtesy of dwm
int error_callback(Display *dpy, XErrorEvent *ee)
{
	if (  ee->error_code == BadWindow ||
		(ee->request_code == X_SetInputFocus     && ee->error_code == BadMatch   ) ||
		(ee->request_code == X_PolyText8         && ee->error_code == BadDrawable) ||
		(ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable) ||
		(ee->request_code == X_PolySegment       && ee->error_code == BadDrawable) ||
		(ee->request_code == X_ConfigureWindow   && ee->error_code == BadMatch   ) ||
		(ee->request_code == X_GrabButton        && ee->error_code == BadAccess  ) ||
		(ee->request_code == X_GrabKey           && ee->error_code == BadAccess  ) ||
		(ee->request_code == X_CopyArea          && ee->error_code == BadDrawable))
		return 0;
	note("fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}
ucell modifier_names_to_mask(char *names)
{
	int i; ucell mod = 0;
	for (i = 0; i < (sizeof(modmasks)/sizeof(struct modmask)); i++)
		if (regmatch(NULL, modmasks[i].pattern, names, 0, NULL, REG_ICASE)) mod |= modmasks[i].mask;
	return mod;
}
struct binding* find_binding(ucell mod, KeyCode key)
{
	int i; struct binding *b = NULL;
	for (i = 0; i < bindings->depth; i++)
	{
		b = bindings->items[i];
		if (b->mod == mod && b->key == key)
			return b;
	}
	return NULL;
}
ubyte create_binding(char *pattern, char *command)
{
	int i; struct binding *b = NULL, *match = NULL;
	for (i = 0; i < bindings->depth; i++)
	{
		match = bindings->items[i];
		if (strcmp(match->pattern, pattern) == 0)
		{
			b = match;
			break;
		}
	}
	ucell mod = modifier_names_to_mask(pattern);
	char *l = pattern, *r = pattern;
	while ((r = strchr(l, '+'))) l = r+1;
	KeySym sym = XStringToKeysym(l);
	KeyCode code = XKeysymToKeycode(display, sym);
	if (!mod || sym == NoSymbol || !code)
		return 0;
	if (!b) b = allocate(sizeof(struct binding));
	strcpy(b->pattern, pattern);
	strcpy(b->command, command);
	b->mod = mod;
	b->key = code;
	stack_push(bindings, b);
	return 1;
}
ubyte remove_binding(char *pattern)
{
	int i; struct binding *match = NULL;
	for (i = 0; i < bindings->depth; i++)
	{
		match = bindings->items[i];
		if (strcmp(match->pattern, pattern) == 0)
		{
			stack_del(bindings, i);
			return 1;
		}
	}
	return 0;
}
void window_property(Window win, Atom prop, char *pad, ucell plen)
{
	XTextProperty tp; int status, n; char** cl; *pad = '\0';
	if (XGetTextProperty(display, win, &tp, prop) != 0)
	{
		status = XmbTextPropertyToTextList(display, &tp, &cl, &n);
		if (status == Success && cl && n > 0)
		{
			int l, i;
			for (i = 0, l = 0; i < n && l < plen; i++)
				l+= snprintf(pad+l, plen-l, "%s\n", cl[i]);
			XFreeStringList(cl);
			strrtrim(pad);
		} else
		if (tp.encoding == XA_STRING)
			snprintf(pad, plen, "%s", (char*)tp.value);
		XFree(tp.value);
	}
}
void window_name(Window win, char *pad)
{
	window_property(win, wmatom[WMName], pad, NOTE);
}
void window_class(Window win, char *pad)
{
	window_property(win, wmatom[WMClass], pad, NOTE);
	char *class = strchr(pad, '\n');
	if (class)
	{
		class++;
		memmove(pad, class, strlen(class)+1);
	}
}
ubyte is_unmanaged_class(char *name)
{
	if (*name)
	{
		int i; for (i = 0; i < unmanaged->depth; i++)
			if (strcmp((char*)(unmanaged->items[i]), name) == 0) return 1;
	}
	return 0;
}
ubyte is_unmanaged_window(Window win)
{
	char name[NOTE];
	window_class(win, name);
	if (strcmp(name, "musca") == 0) return 1;
	return is_unmanaged_class(name);
}
ubyte window_on_screen(head *hd, Window win, XWindowAttributes *attr, int *x, int *y, int *w, int *h)
{
	XWindowAttributes _attr;
	if (!attr)
	{
		attr = &_attr;
		XGetWindowAttributes(display, win, attr);
	}
	int lx = attr->x, ly = attr->y, lw = attr->width, lh = attr->height;
	if (lx + lw > hd->screen->width)  lx -= (lx + lw - hd->screen->width);
	if (ly + lh > hd->screen->height) ly -= (ly + lh - hd->screen->height);
	ubyte on_scr = (lx == attr->x && ly == attr->y) ? 1: 0;
	*x = lx; *y = ly; *w = lw; *h = lh;
	return on_scr;
}
Window window_create(Window parent, int x, int y, int w, int h, int bw, char *bc, char *bg, char *name, char *class)
{
	Window win = XCreateSimpleWindow(display, parent, x, y, w, h, bw,
		bc ? get_color(heads, bc): None, bg ? get_color(heads, bg): None);
	if (name)
	{
		XClassHint *hint = XAllocClassHint(); hint->res_name = name; hint->res_class = class;
		XSetClassHint(display, win, hint);
		XFree(hint);
	}
	return win;
}

// SEARCHES

// loop once around a doubly-linked ring
#define FOR_RING(v,f,i) for ((v) = (f), (i) = 0; (v) && (!(i) || (v) != (f)); (v) = (v)->next, (i)++)

head* head_by_screen(Screen *s)
{
	head *f; int i;
	FOR_RING (f, heads, i)
		if (f->screen == s) return f;
	wtf("unknown screen");
	return NULL;
}
client* client_by_window(Window w)
{
	head *h; group *g; client *c; int i, j, k;
	FOR_RING (h, heads, i)
		FOR_RING (g, h->groups, j)
			FOR_RING (c, g->clients, k)
				if (c->win == w) return c;
	return NULL;
}
client* client_by_resize_window(Window w)
{
	head *h; group *g; client *c; int i, j, k;
	FOR_RING (h, heads, i)
		FOR_RING (g, h->groups, j)
			FOR_RING (c, g->clients, k)
				if (c->rl == w) return c;
	return NULL;
}
client* client_hidden(group *t)
{
	client *c; int i;
	FOR_RING (c, t->clients, i)
		if (!c->frame || (c->frame->cli != c && !c->kids)) return c;
	return NULL;
}
frame* is_frame_background(Window win)
{
	int i, j, k;
	head *h; group *g; frame *f;
	FOR_RING (h, heads, i)
		FOR_RING (g, h->groups, j)
			FOR_RING (f, g->frames, k)
				if (f->win == win) return f;
	return NULL;
}
ubyte is_valid_client(client *c)
{
	int i, j, k;
	head *h; group *g; client *d;
	FOR_RING (h, heads, i)
		FOR_RING (g, h->groups, j)
			FOR_RING (d, g->clients, k)
				if (d == c) return 1;
	return 0;
}

// CLIENTS

void client_configure(client *c, XConfigureRequestEvent *cr)
{
	XWindowAttributes attr;
	Window trans = None;
	XSizeHints hints;
	long userhints;
	int x, y, w, h, pw, ph, b;
	frame *f = c->frame;
	group *g = c->group;
	Window win = c->win;
	char debug[BLOCK]; int dlen = 0;

	if (g->flags & GF_STACKING)
	{
		x = g->l; y = g->t;
		pw = g->head->screen->width - g->l - g->r;
		ph = g->head->screen->height - g->t - g->b;
	} else
	{
		x = f->x+1; y = f->y+1;
		pw = f->w-2; ph = f->h-2;
	}
	w = pw; h = ph;
	window_name(c->win, c->name);
	window_class(c->win, c->class);
	dlen += sprintf(debug+dlen, "%s ", c->name);

	if (!XGetTransientForHint(display, win, &trans) && trans == None && g->flags & GF_TILING)
	{
		// normal windows go full frame
		dlen += sprintf(debug+dlen, "normal ");
		w = pw; h = ph; b = 0;
	} else
	{
		b = 1;
		// stacking and transient windows stay their preferred size
		if (XGetWindowAttributes(display, win, &attr))
		{
			x = attr.x; y = attr.y; w = attr.width; h = attr.height;
		}
		if (cr)
		{
			if (cr->value_mask & CWX) x = cr->x;
			if (cr->value_mask & CWY) y = cr->y;
			if (cr->value_mask & CWWidth)  w = cr->width;
			if (cr->value_mask & CWHeight) h = cr->height;
		}
		dlen += sprintf(debug+dlen, "floating %d %d %d %d ", x, y, w, h);
	}
	if (XGetWMNormalHints(display, win, &hints, &userhints))
	{
		dlen += sprintf(debug+dlen, "hints ");
		if (hints.flags & PMinSize)
		{
			dlen += sprintf(debug+dlen, "PMinSize %d %d ", hints.min_width, hints.min_height);
			w = MAX(w, hints.min_width); h = MAX(h, hints.min_height);
		}
		if (hints.flags & PMaxSize)
		{
			dlen += sprintf(debug+dlen, "PMaxSize %d %d ", hints.max_width, hints.max_height);
			w = MIN(w, hints.max_width); h = MIN(h, hints.max_height);
		}
		if (hints.flags & PResizeInc && hints.flags & PBaseSize)
		{
			w -= hints.base_width; h -= hints.base_height;
			w -= w % hints.width_inc; h -= h % hints.height_inc;
			w += hints.base_width; h += hints.base_height;
			dlen += sprintf(debug+dlen, "PResizeInc %d %d %d %d ", hints.width_inc, hints.height_inc, w, h);
		}
		if (hints.flags & PAspect)
		{
			dcell ratio = (dcell)w / h;
			dcell minr = (dcell)hints.min_aspect.x / hints.min_aspect.y;
			dcell maxr = (dcell)hints.max_aspect.x / hints.max_aspect.y;
			dlen += sprintf(debug+dlen, "PAspect %2.2f %2.2f %2.2f ", ratio, minr, maxr);
				if (ratio < minr) h = (int)(w / minr);
			else if (ratio > maxr) w = (int)(h * maxr);
		}
		w = MIN(w, pw); h = MIN(h, ph);
		if (trans == None)
		{
			w = MAX(settings[ms_frame_min_wh].u-2, w);
			h = MAX(settings[ms_frame_min_wh].u-2, h);
		}
	}
	// center fixed size window whe tiling
	if (g->flags & GF_TILING && (w < pw || h < ph))
	{
		x = f->x+1 + ((pw - w) / 2);
		y = f->y+1 + ((ph - h) / 2);
	}
	dlen += sprintf(debug+dlen, "Co-ords %d %d %d %d ", x, y, w, h);
	if (c->x != x || c->y != y || c->w != w || c->h != h)
		XMoveResizeWindow(display, win, x, y, w, h);
	XSetWindowBorderWidth(display, win, b);
	XSetWindowBorder(display, win, get_color(c->group->head, settings[ms_border_unfocus].s));
	c->x = x; c->y = y; c->w = w; c->h = h;
	if (DEBUG) note("%s", debug);
}
void client_display(client *c, frame *target)
{
	if (target) c->frame = target;
	if (c->frame) c->frame->cli = c;
	client_configure(c, NULL);
	c->flags &= ~CF_HIDDEN;
	XMapWindow(display, c->win);
	XRaiseWindow(display, c->win);
	if (c->frame && c->group->flags & GF_TILING)
	{
		Window wins[3]; int i = 0;
		wins[i++] = c->win;
		if (c->parent) wins[i++] = c->parent->win;
		wins[i++] = c->frame->win;
		XRestackWindows(display, wins, i);
	}
}
void client_unfocus(client *c)
{
	XSetWindowBorder(display, c->win, get_color(c->group->head, settings[ms_border_unfocus].s));
}
void client_focus(client *c, frame *target)
{
	if (c->group->clients) client_unfocus(c->group->clients);
	client_display(c, target); c->group->clients = c;
	XSetInputFocus(display, c->win, RevertToPointerRoot, CurrentTime);
	XSetWindowBorder(display, c->win, get_color(c->group->head, settings[ms_border_focus].s));
}
void client_hide(client *c)
{
	c->flags |= CF_HIDDEN;
	XUnmapWindow(display, c->win);
}
void client_remove(client *c)
{
	frame *f = c->frame;
	client *p = c->parent;
	client_destroy(c);
	if (!f->cli)
	{
		ubyte focus = heads->groups->frames == f ? 1: 0;
		if (p && is_valid_client(p) && (!p->frame || p->frame->cli != p))
		{
			if (focus) client_focus(p, f);
			else client_display(p, f);
		} else
		if (focus) frame_focus_hidden(f);
		else frame_display_hidden(f);
	}
}
void client_push(group *t, client *c)
{
	c->group = t;
	if (t->clients)
	{
		client *tmp = t->clients->prev;
		t->clients->prev = c;
		tmp->next = c;
		c->prev = tmp;
		c->next = t->clients;
	} else
	{
		t->clients = c;
		c->next = c;
		c->prev = c;
	}
}
void client_pop(client *c)
{
	group *t = c->group;
	if (t->clients == t->clients->next)
		t->clients = NULL;
	else
	{
		c->next->prev = c->prev;
		c->prev->next = c->next;
		if (t->clients == c)
			t->clients = c->next;
	}
	if (c->frame && c->frame->cli == c)
		c->frame->cli = NULL;
}
client* client_create(group *t, frame *f, Window win)
{
	client *c = allocate(sizeof(client));
	c->frame = f; c->win = win; c->flags = CF_HIDDEN;
	c->group = NULL; c->next = NULL; c->prev = NULL;
	c->kids = 0; c->parent = NULL;
	c->name[0] = '\0'; c->class[0] = '\0';
	c->x = 0; c->y = 0; c->w = 0; c->h = 0;
	c->fx = 0; c->fy = 0; c->fw = 0; c->fh = 0;
	c->rl = 0; c->rr = 0; c->rt = 0; c->rb = 0;
	if (t) client_push(t, c);
	return c;
}
void client_destroy(client *c)
{
	client_pop(c);
	if (c->parent && is_valid_client(c->parent))
		c->parent->kids--;
	free(c);
}
void client_regroup(group *g, client *c)
{
	if (c->kids)
	{
		client *k; int i;
		FOR_RING (k, c->group->clients, i)
			if (k->parent == c) client_regroup(g, k);
	}
	client_pop(c); client_hide(c); client_push(g, c);
}

// FRAMES

#define FRAME_REPORT(f) note("screen %d xywh %d %d %d %d", (f)->group->head->id, (f)->x, (f)->y, (f)->w, (f)->h)

void frame_push(group *t, frame *f)
{
	f->group = t;
	if (t->frames)
	{
		frame *tmp = t->frames->prev;
		t->frames->prev = f;
		tmp->next = f;
		f->prev = tmp;
		f->next = t->frames;
	} else
	{
		t->frames = f;
		f->next = f;
		f->prev = f;
	}
}
void frame_pop(frame *f)
{
	group *t = f->group;
	if (t->frames == t->frames->next)
		t->frames = NULL;
	else
	{
		f->next->prev = f->prev;
		f->prev->next = f->next;
		if (t->frames == f)
			t->frames = f->next;
	}
	client *c; int i;
	FOR_RING (c, t->clients, i)
		if (c->frame && c->frame == f)
			c->frame = t->frames;
}
ucell frame_border_focus(frame *f)
{
	char *color = NULL;
	if (f->group->flags & GF_TILING)
	{
		color = settings[ms_border_focus].s;
		if (f->flags & FF_DEDICATE) color = settings[ms_border_dedicate_focus].s;
		if (f->flags & FF_CATCHALL) color = settings[ms_border_catchall_focus].s;
	}
	return color ? get_color(f->group->head, color): None;
}
ucell frame_border_unfocus(frame *f)
{
	char *color = NULL;
	if (f->group->flags & GF_TILING)
	{
		color = settings[ms_border_unfocus].s;
		if (f->flags & FF_DEDICATE) color = settings[ms_border_dedicate_unfocus].s;
		if (f->flags & FF_CATCHALL) color = settings[ms_border_catchall_unfocus].s;
	}
	return color ? get_color(f->group->head, color): None;
}
frame* frame_create(group *t, int x, int y, int w, int h)
{
	frame *f = allocate(sizeof(frame));
	f->x = x; f->y = y; f->w = w; f->h = h; f->flags = 0;
	f->group = NULL; f->cli = NULL; f->next = NULL; f->prev = NULL;
	if (t) frame_push(t, f);
	// frames have a background window, just for borders so far
	f->win = window_create(t->head->screen->root, x, y, w-2, h-2, 1,
		settings[ms_border_unfocus].s, NULL, "musca", "musca");
	// pseudo-transparent to the root window
	XSetWindowBackgroundPixmap(display, f->win, ParentRelative);
	return f;
}
void frame_destroy(frame *f)
{
	frame_pop(f);
	XDestroyWindow(display, f->win);
	free(f);
}
frame* frame_available(frame *f)
{
	frame *a, *b; int i;
	// is there a catchall frame
	FOR_RING (a, f, i)
		if (a->flags & FF_CATCHALL) return a;
	// if current frame is empty, use it
	if (!f->cli) return f;
	// locate undedicated frame, and preferring current frame
	FOR_RING (a, f, i)
		if (!(a->flags & FF_DEDICATE)) break;
	if (a != f)
	{
		// we have to switch frames, so step back and find an empty one
		FOR_RING (b, f, i)
			if (!(b->flags & FF_DEDICATE) && !b->cli) break;
		if (b != f) a = b;
	}
	// at this stage just take the current frame regardless
	if (a->flags & FF_DEDICATE)
		frame_dedicate(a);
	return a;
}
void frame_display_hidden(frame *f)
{
	client *c = client_hidden(f->group);
	if (c) client_display(c, f);
}
void frame_focus_hidden(frame *f)
{
	client *c = client_hidden(f->group);
	if (c) client_focus(c, f);
	if (!f->cli) XSetInputFocus(display, f->win, RevertToPointerRoot, CurrentTime);
}
void frames_display_hidden(group *t)
{
	frame *f; int i;
	FOR_RING (f, t->frames, i)
		if (!f->cli || (f->cli && f->cli->frame != f))
			frame_display_hidden(f);
}
// any changes to frames setting do not take effect until this is called
// bundle multiple changes before a single call to prevent flicker
void frame_update(frame *f)
{
	XSetWindowBackgroundPixmap(display, f->win, ParentRelative);
	XMoveResizeWindow(display, f->win, f->x, f->y, f->w-2, f->h-2);
	XMapWindow(display, f->win);
	if (f->cli && f->cli->frame == f)
		client_display(f->cli, NULL);
	else	frame_display_hidden(f);
}
void frame_unfocus(frame *f)
{
	if (f->cli) client_unfocus(f->cli);
	XSetWindowBorder(display, f->win, frame_border_unfocus(f));
}
void frame_focus(frame *f)
{
	head *h = f->group->head;
	if (f != h->groups->frames)
		frame_unfocus(h->groups->frames);
	h->groups->frames = f;
	XSetWindowBorder(display, f->win, frame_border_focus(f));
	if (f->group->flags & GF_STACKING)
	{
		// display everything
		client *c; int i;
		FOR_RING (c, f->group->clients, i)
			if (c->flags & CF_HIDDEN) client_display(c, f);
		if (c) client_focus(c, f);
	} else
	{
		// display only one
		if (f->cli && f->cli->frame == f)
			client_focus(f->cli, NULL);
		else frame_focus_hidden(f);
	}
}
void frame_hide(frame *f)
{
	XUnmapWindow(display, f->win);
}
void frame_hsplit(dcell ratio)
{
	frame *f = heads->groups->frames;
	group *t = f->group;
	int lw = ceil(f->w * ratio);
	if (ratio >= 1 || lw < settings[ms_frame_min_wh].u || f->w - lw < settings[ms_frame_min_wh].u)
	{
		say("unable to split: %f %d %d", ratio, lw, f->w - lw);
		return;
	}
	group_track(heads->groups);
	int rw = f->w - lw, rx = f->x + lw;
	f->w -= rw; frame_update(f);
	frame *n = frame_create(t, rx, f->y, rw, f->h);
	frame_update(n);
}
void frame_hsplit_half()
{
	frame_hsplit(0.5);
}
void frame_vsplit(dcell ratio)
{
	frame *f = heads->groups->frames;
	group *t = f->group;
	int th = ceil(f->h * ratio);
	if (ratio >= 1 || th < settings[ms_frame_min_wh].u || f->h - th < settings[ms_frame_min_wh].u)
	{
		say("unable to split: %f %d %d", ratio, th, f->h - th);
		return;
	}
	group_track(heads->groups);
	int bh = f->h - th, by = f->y + th;
	f->h -= bh; frame_update(f);
	frame *n = frame_create(t, f->x, by, f->w, bh);
	frame_update(n);
}
void frame_vsplit_half()
{
	frame_vsplit(0.5);
}
void frame_split(ubyte direction, dcell ratio)
{
	if (direction == VERTICAL)
		frame_vsplit(ratio);
	else frame_hsplit(ratio);
}
// return 0/false if the frame is not adjacent to the block defined by x, y, w, h.
// if a frame side does touch a block side, and the length of the frame's side is
// contained *entirely within* the block side, return the side number.
// also see frame_relative()
ubyte frame_borders(frame *s, int x, int y, int w, int h)
{
	ubyte in_y = (s->y >= y && s->y + s->h <= y + h) ? 1: 0;
	ubyte in_x = (s->x >= x && s->x + s->w <= x + w) ? 1: 0;
	if (s->x == x && in_y) return LEFT;
	if (s->x == x + w && in_y) return LEFT;
	if (s->x + s->w == x && in_y) return RIGHT;
	if (s->x + s->w == x + w && in_y) return RIGHT;
	if (s->y == y && in_x) return TOP;
	if (s->y == y + h && in_x) return TOP;
	if (s->y + s->h == y && in_x) return BOTTOM;
	if (s->y + s->h == y + h && in_x) return BOTTOM;
	return 0;
}
// locate all frames bordering the block defined by x, y, w, h.  'bordering'
// follows the frame_borders() rules.
struct frame_match* frames_bordering(group *t, int x, int y, int w, int h)
{
	// null terminator
	frame *f; int i = 0, fc;
	FOR_RING (f, t->frames, fc); fc++;
	struct frame_match *matches = allocate(sizeof(struct frame_match)*fc);
	memset(matches, 0, sizeof(struct frame_match)*fc);
	FOR_RING (f, t->frames, fc)
	{
		ubyte side = frame_borders(f, x, y, w, h);
		if (side)
		{
			matches[i].frame = f;
			matches[i].side = side;
			i++;
		}
	}
	return matches;
}
ubyte frame_in_set(frame **set, frame *f)
{
	int i = 0; if (!set) return 0;
	while (set[i] && set[i] != f) i++;
	return set[i] ? 1: 0;
}
// resize as many frames as necessary to fill the block defined by x, y, w, h, ignoring
// any frames in **exceptions.
void frames_fill_gap_except(group *t, frame **exceptions, int x, int y, int w, int h, ubyte mode)
{
	struct frame_match *matches = frames_bordering(t, x, y, w, h);
	int sides[5], i; memset(&sides, 0, sizeof(int)*5);
	for (i = 0; matches[i].frame; i++)
		if (!frame_in_set(exceptions, matches[i].frame))
			sides[matches[i].side]++;
	if (sides[LEFT] && sides[RIGHT])
		sides[sides[LEFT] < sides[RIGHT] ? RIGHT: LEFT] = 0;
	if (sides[TOP] && sides[BOTTOM])
		sides[sides[TOP] < sides[BOTTOM] ? BOTTOM: TOP] = 0;
	if (mode == FRAMES_FEWEST)
	{
		if (sides[TOP] + sides[BOTTOM] < sides[LEFT] + sides[RIGHT])
			{ sides[TOP] = 0; sides[BOTTOM] = 0; }
		else { sides[LEFT] = 0; sides[RIGHT] = 0; }
	}
	for (i = 0; matches[i].frame; i++)
	{
		frame *f = matches[i].frame;
		ubyte side = matches[i].side;
		if (frame_in_set(exceptions, f)) continue;
		if (!sides[side]) continue;
		     if (side == LEFT) { f->x -= w; f->w += w; }
		else if (side == RIGHT)  f->w += w;
		else if (side == TOP)  { f->y -= h; f->h += h; }
		else if (side == BOTTOM) f->h += h;
		frame_update(f);
	}
	free(matches);
}
void frames_fill_gap(group *t, int x, int y, int w, int h, ubyte mode)
{
	frames_fill_gap_except(t, NULL, x, y, w, h, mode);
}
ubyte nearest_side(int x, int y, int a, int b, int c, int d)
{
	int left = x - a, right = c - x, top = y - b, bottom = d - y;
	if (right < left && right < top && right < bottom) return RIGHT;
	if (top < left && top < bottom) return TOP;
	if (bottom < left) return BOTTOM;
	return LEFT;
}
// return false/0 if the frame does not cover the block dfine dby x, y, w, h.
// if the frame and block do overlap anywhere, return the side of the frame
// closest to a block side.
ubyte frame_covers(frame *s, int x, int y, int width, int height)
{
	ubyte side = 0;
	int a = x, b = y, c = x + width, d = y + height;
	int e = s->x, f = s->y, g = s->x + s->w, h = s->y + s->h;
	     if (e > a && f > b && e < c && f < d) // ef
		side = nearest_side(e, f, a, b, c, d);
	else if (g > a && f > b && g < c && f < d) // gf
		side = nearest_side(g, f, a, b, c, d);
	else if (e > a && h > b && e < c && h < d) // eh
		side = nearest_side(e, h, a, b, c, d);
	else if (g > a && h > b && g < c && h < d) // gh
		side = nearest_side(g, h, a, b, c, d);
	else if (a > e && b > f && a < g && b < h) // ab
		side = nearest_side(a, b, e, f, g, h);
	else if (c > e && b > f && c < g && b < h) // cb
		side = nearest_side(c, b, e, f, g, h);
	else if (a > e && d > f && a < g && d < h) // ad
		side = nearest_side(a, d, e, f, g, h);
	else if (c > e && d > f && c < g && d < h) // cd
		side = nearest_side(c, d, e, f, g, h);
	return side;
}
// resize any frames obscuring the block defined by x, y, width, height, ignoring
// any frames in **exceptions.
void frames_make_gap_except(group *t, frame **exceptions, int x, int y, int width, int height)
{
	frame *s; int i;
	FOR_RING (s, t->frames, i)
	{
		ubyte side = 0;
		if (!frame_in_set(exceptions, s))
		{
			side = frame_covers(s, x, y, width, height);
			int inc = 0;
			     if (side == LEFT) { inc = x + width - s->x; s->x += inc; s->w -= inc; }
			else if (side == RIGHT)  s->w -= (s->x + s->w) - x;
			else if (side == TOP)  { inc = y + height - s->y; s->y += inc; s->h -= inc; }
			else if (side == BOTTOM) s->h -= (s->y + s->h) - y;
			else {
				side = frame_borders(s, x, y, width, height);
				     if (side == LEFT) { s->x += width; s->w -= width; }
				else if (side == RIGHT)  s->w -= width;
				else if (side == TOP)  { s->y += height; s->h -= height; }
				else if (side == BOTTOM) s->h -= height;
			}
		}
		if (side) frame_update(s);
	}
}
void frames_make_gap(group *t, int x, int y, int w, int h)
{
	frames_make_gap_except(t, NULL, x, y, w, h);
}
// look for a frame that has a side on the same 'axis' as the supplied frame, *and* has
// 'side' adjacent to the supplied frame.
frame* frame_sibling(frame *f, ubyte axis, ubyte side)
{
	frame *s; int i;
	// dont return a sibling if a frame opposite the 'axis' has an opposing side in line with 'side'
	FOR_RING (s, f->group->frames, i)
	{
		if (s != f && (
			(side == LEFT && s->x + s->w == f->x &&
				((axis == TOP && s->y + s->h == f->y) || (axis == BOTTOM && s->y == f->y + f->h))) ||
			(side == RIGHT && s->x == f->x + f->w &&
				((axis == TOP && s->y + s->h == f->y) || (axis == BOTTOM && s->y == f->y + f->h))) ||
			(side == TOP && s->y + s->h == f->y &&
				((axis == LEFT && s->x + s->w == f->x) || (axis == RIGHT && s->x == f->x + f->w))) ||
			(side == BOTTOM && s->y == f->y + f->h &&
				((axis == LEFT && s->x + s->w == f->x) || (axis == RIGHT && s->x == f->x + f->w)))
			)) return NULL;
	}
	// return a sibling if a frame is on the same side of the 'axis' has an oppsoing side abutting 'side'
	FOR_RING (s, f->group->frames, i)
	{
		if (s != f && (
			(axis == LEFT   && f->x == s->x) ||
			(axis == RIGHT  && f->x + f->w == s->x + s->w) ||
			(axis == TOP    && f->y == s->y) ||
			(axis == BOTTOM && f->y + f->h == s->y + s->h)
			) && (
			(side == LEFT   && f->x == s->x + s->w) ||
			(side == RIGHT  && f->x + f->w == s->x) ||
			(side == TOP    && f->y == s->y + s->h) ||
			(side == BOTTOM && f->y + f->h == s->y)
			)) return s;
	}
	return NULL;
}
// find a frame's siblings on the same axis.  these will form a single block bordering the axis
frame** frame_siblings(frame *f, ubyte axis)
{
	frame *s, **matches = allocate(sizeof(frame*)*NOTE);
	memset(matches, 0, sizeof(frame*)*NOTE);
	// first check for an opposite frame that exactly matches. we don't
	// want to move a group unless we have to
	int i = 0; matches[i++] = f;
	ubyte side1 = 0, side2 = 0;
	if (axis == LEFT || axis == RIGHT)
		{ side1 = TOP; side2 = BOTTOM; }
	else { side1 = LEFT; side2 = RIGHT; }
	s = f->group->frames;
	do {	s = frame_sibling(s, axis, side1);
		if (s) matches[i++] = s;
	} while (s);
	s = f;
	do {	s = frame_sibling(s, axis, side2);
		if (s) matches[i++] = s;
	} while (s);
	return matches;
}
// check whether a group of frames is resizable in the direction of 'axis'
ubyte frame_siblings_growable(frame **siblings, ubyte axis, ucell size)
{
	int i, x, y, w, h;
	frame *f = siblings[0];
	if (axis == LEFT || axis == RIGHT)
	{
		if (axis == LEFT) { x = f->x - size; y = f->y; }
		else { x = f->x + f->w; y = f->y; }
		w = size; h = 0;
		for (i = 0; siblings[i]; i++)
			y = MIN(y, siblings[i]->y), h += siblings[i]->h;
	} else
	if (axis == TOP || axis == BOTTOM)
	{
		if (axis == TOP) { x = f->x; y = f->y - size; }
		else { x = f->x; y = f->y + f->h; }
		w = 0; h = size;
		for (i = 0; siblings[i]; i++)
			x = MIN(x, siblings[i]->x), w += siblings[i]->w;
	}
	FOR_RING (f, f->group->frames, i)
	{
		if (frame_covers(f, x+1, y+1, w-2, h-2))
		{
			if ((axis == LEFT || axis == RIGHT ) && f->w <= settings[ms_frame_min_wh].u) return 0;
			if ((axis == TOP  || axis == BOTTOM) && f->h <= settings[ms_frame_min_wh].u) return 0;
		}
	}
	return 1;
}
void frame_remove()
{
	frame *f = heads->groups->frames;
	group *t = f->group;
	if (t->frames->next == f) return;
	group_track(t);
	frame *set[2]; set[0] = f; set[1] = NULL;
	frames_fill_gap_except(f->group, set, f->x, f->y, f->w, f->h, FRAMES_FEWEST);
	frame_destroy(f);
	frame_focus(t->frames);
}
ubyte frame_shrink(ubyte direction, ubyte adapt, ucell size)
{
	group *t = heads->groups;
	frame *f = t->frames;
	frame **group = NULL;
	int i = 0, x, y, w, h;
	if (direction == HORIZONTAL)
	{
		if (f->x + f->w >= t->head->screen->width - t->r)
		{
			if (adapt) return frame_grow(direction, 0, size);
			else {
				if (f->w <= settings[ms_frame_min_wh].u || t->frames->next == f || f->x <= t->l) return 0;
				group = frame_siblings(f, LEFT);
				x = f->x; w = size; y = f->y; h = 0;
				for (i = 0; group[i]; i++)
					y = MIN(y, group[i]->y), h += group[i]->h;
				frames_fill_gap_except(f->group, group, x, y, w, h, FRAMES_FEWEST);
				for (i = 0; group[i]; i++)
				{
					group[i]->w -= size, group[i]->x += size;
					frame_update(group[i]);
				}
			}
		} else
		{
			if (f->w <= settings[ms_frame_min_wh].u || t->frames->next == f) return 0;
			group = frame_siblings(f, RIGHT);
			x = f->x + f->w - size; w = size; y = f->y; h = 0;
			for (i = 0; group[i]; i++)
				y = MIN(y, group[i]->y), h += group[i]->h;
			frames_fill_gap_except(f->group, group, x, y, w, h, FRAMES_FEWEST);
			for (i = 0; group[i]; i++)
			{
				group[i]->w -= size;
				frame_update(group[i]);
			}
		}
	} else
	if (direction == VERTICAL)
	{
		if (f->y + f->h >= t->head->screen->height - t->b)
		{
			if (adapt) return frame_grow(direction, 0, size);
			else {
				if (f->h <= settings[ms_frame_min_wh].u || t->frames->next == f || f->y <= t->t) return 0;
				group = frame_siblings(f, TOP);
				x = f->x; w = 0; y = f->y; h = size;
				for (i = 0; group[i]; i++)
					x = MIN(x, group[i]->x), w += group[i]->w;
				frames_fill_gap_except(f->group, group, x, y, w, h, FRAMES_FEWEST);
				for (i = 0; group[i]; i++)
				{
					group[i]->h -= size, group[i]->y += size;
					frame_update(group[i]);
				}
			}
		} else
		{
			if (f->h <= settings[ms_frame_min_wh].u || t->frames->next == f) return 0;
			group = frame_siblings(f, BOTTOM);
			y = f->y + f->h - size; h = size; x = f->x; w = 0;
			for (i = 0; group[i]; i++)
				x = MIN(x, group[i]->x), w += group[i]->w;
			frames_fill_gap_except(f->group, group, x, y, w, h, FRAMES_FEWEST);
			for (i = 0; group[i]; i++)
			{
				group[i]->h -= size;
				frame_update(group[i]);
			}
		}
	}
	if (group) free(group);
	frame_focus(f);
	return 1;
}
ubyte frame_grow(ubyte direction, ubyte adapt, ucell size)
{
	group *t = heads->groups;
	frame *f = t->frames;
	frame **group = NULL;
	int i = 0, x, y, w, h;
	ubyte changes = 0;
	if (direction == HORIZONTAL)
	{
		if (f->x + f->w >= t->head->screen->width - t->r)
		{
			if (adapt) return frame_shrink(direction, 0, size);
			else {
				if (t->frames->next == f || f->x <= t->l) return 0;
				group = frame_siblings(f, LEFT);
				if (frame_siblings_growable(group, LEFT, size))
				{
					changes = 1;
					x = f->x - size; w = size; y = f->y; h = 0;
					for (i = 0; group[i]; i++)
						y = MIN(y, group[i]->y), h += group[i]->h;
					frames_make_gap_except(f->group, group, x, y, w, h);
					for (i = 0; group[i]; i++)
					{
						group[i]->w += size, group[i]->x -= size;
						frame_update(group[i]);
					}
				}
			}
		} else
		{
			if (t->frames->next == f) return 0;
			group = frame_siblings(f, RIGHT);
			if (frame_siblings_growable(group, RIGHT, size))
			{
				changes = 1;
				x = f->x + f->w; w = size; y = f->y; h = 0;
				for (i = 0; group[i]; i++)
					y = MIN(y, group[i]->y), h += group[i]->h;
				frames_make_gap_except(f->group, group, x, y, w, h);
				for (i = 0; group[i]; i++)
				{
					group[i]->w += size;
					frame_update(group[i]);
				}
			}
		}
	} else
	if (direction == VERTICAL)
	{
		if (f->y + f->h >= t->head->screen->height - t->b)
		{
			if (adapt) return frame_shrink(direction, 0, size);
			else {
				if (t->frames->next == f || f->y <= t->t) return 0;
				group = frame_siblings(f, TOP);
				if (frame_siblings_growable(group, TOP, size))
				{
					changes = 1;
					y = f->y - size; h = size; x = f->x; w = 0;
					for (i = 0; group[i]; i++)
						x = MIN(x, group[i]->x), w += group[i]->w;
					frames_make_gap_except(f->group, group, x, y, w, h);
					for (i = 0; group[i]; i++)
					{
						group[i]->h += size, group[i]->y -= size;
						frame_update(group[i]);
					}
				}
			}
		} else
		{
			if (t->frames->next == f) return 0;
			group = frame_siblings(f, BOTTOM);
			if (frame_siblings_growable(group, BOTTOM, size))
			{
				changes = 1;
				y = f->y + f->h; h = size; x = f->x; w = 0;
				for (i = 0; group[i]; i++)
					x = MIN(x, group[i]->x), w += group[i]->w;
				frames_make_gap_except(f->group, group, x, y, w, h);
				for (i = 0; group[i]; i++)
				{
					group[i]->h += size;
					frame_update(group[i]);
				}
			}
		}
	}
	if (group) free(group);
	frame_focus(f);
	return 1;
}
void frame_only()
{
	group_track(heads->groups);
	group *g = heads->groups;
	frame *f = g->frames;
	while (f != f->next)
		frame_destroy(f->next);
	f->x = g->l; f->y = g->t;
	f->w = g->head->screen->width  - g->r - g->l;
	f->h = g->head->screen->height - g->b - g->t;
	frame_update(f);
	frame_focus(f);
}
char frames_overlap_y(frame *a, frame *b)
{
	return ((a->y <= b->y && (a->y + a->h) > b->y) || (b->y <= a->y && (b->y + b->h) > a->y))
		? 1: 0;
}
char frames_overlap_x(frame *a, frame *b)
{
	return ((a->x <= b->x && (a->x + a->w) > b->x) || (b->x <= a->x && (b->x + b->w) > a->x))
		? 1 : 0;
}
// find a frame that has some adjacent border to the supplied frame, in the direction of side
// this bails out with the first find.  could be expanded to return the frame with the longest
// stretch of adjacent border, which might be more intuitive in the UI navigation.
frame* frame_relative(frame *f, ubyte side)
{
	frame *s; int i;
	FOR_RING (s, f, i)
	{
		// skip self
		if (!i) continue;
		if (	(side == LEFT   && f->x == (s->x + s->w) && frames_overlap_y(f, s)) ||
			(side == RIGHT  && s->x == (f->x + f->w) && frames_overlap_y(f, s)) ||
			(side == TOP    && f->y == (s->y + s->h) && frames_overlap_x(f, s)) ||
			(side == BOTTOM && s->y == (f->y + f->h) && frames_overlap_x(f, s))
			) return s;
	}
	return NULL;
}
void frame_left()
{
	frame *s = frame_relative(heads->groups->frames, LEFT);
	if (s) frame_focus(s);
}
void frame_right()
{
	frame *s = frame_relative(heads->groups->frames, RIGHT);
	if (s) frame_focus(s);
}
void frame_up()
{
	frame *s = frame_relative(heads->groups->frames, TOP);
	if (s) frame_focus(s);
}
void frame_down()
{
	frame *s = frame_relative(heads->groups->frames, BOTTOM);
	if (s) frame_focus(s);
}
void frame_kill_client()
{
	frame *f = heads->groups->frames;
	if (f->cli)
	{
		if (f->cli->flags & CF_KILLED)
		{
			// we already sent a close event and apparently got ignored, so get nasty
			XKillClient(display, f->cli->win);
			client_remove(f->cli);
		} else
		{
			// be polite the by dfault
			XEvent ke;
			ke.type = ClientMessage;
			ke.xclient.window = f->cli->win;
			ke.xclient.message_type = wmatom[WMProtocols];
			ke.xclient.format = 32;
			ke.xclient.data.l[0] = wmatom[WMDelete];
			ke.xclient.data.l[1] = CurrentTime;
			XSendEvent(display, f->cli->win, False, NoEventMask, &ke);
			f->cli->flags |= CF_KILLED;
		}
	}
}
// focus the next hidden client
void frame_cycle_client()
{
	frame_focus_hidden(heads->groups->frames);
}
// exchange the content of the current frame for another
void frame_swap(ubyte direction)
{
	group_track(heads->groups);
	group *t = heads->groups;
	frame *f = t->frames;
	frame *s = frame_relative(f, direction);
	if (s)
	{
		client *a = f->cli, *b = s->cli;
		f->cli = NULL; s->cli = NULL;
		if (a) client_display(a, s);
		if (b) client_display(b, f);
	}
	frame_focus(f);
}
void frame_dedicate()
{
	frame *f = heads->groups->frames;
	if (!(f->flags & FF_CATCHALL))
	{
		if (f->flags & FF_DEDICATE)
			f->flags &= ~FF_DEDICATE;
		else	f->flags |= FF_DEDICATE;
	}
	frame_focus(f);
}
void frame_catchall()
{
	frame *a, *f = heads->groups->frames;
	if (f->flags & FF_CATCHALL)
		f->flags &= ~FF_CATCHALL;
	else {
		for (a = f->next; a != f; a = a->next)
		{
			// there can only be one
			if (a->flags & FF_CATCHALL)
			{
				a->flags &= ~FF_CATCHALL;
				frame_unfocus(f);
			}
		}
		f->flags |= FF_CATCHALL;
		// catchall means undedicated
		f->flags &= ~FF_DEDICATE;
	}
	frame_focus(f);
}

// GROUPS

#define GROUP_REPORT(group) note("screen %d group %s pad %d %d %d %d", (group)->head->id, (group)->name, (group)->l, (group)->r, (group)->t, (group)->b)

void group_push(head *h, group *t)
{
	t->head = h;
	if (h->groups)
	{
		group *tmp = h->groups->prev;
		h->groups->prev = t;
		tmp->next = t;
		t->prev = tmp;
		t->next = h->groups;
	} else
	{
		h->groups = t;
		t->next = t;
		t->prev = t;
	}
}
void group_pop(group *t)
{
	head *h = t->head;
	if (h->groups == h->groups->next)
		h->groups = NULL;
	else {
		t->next->prev = t->prev;
		t->prev->next = t->next;
		if (h->groups == t)
			h->groups = t->next;
	}
}
group* group_create(head *head, char *name, int x, int y, int w, int h)
{
	group *t = allocate(sizeof(group));
	t->frames = NULL; t->clients = NULL; t->flags = GF_TILING;
	t->head = NULL; t->next = NULL; t->prev = NULL;
	t->l = 0; t->r = 0; t->t = 0; t->b = 0;
	strcpy(t->name, name); t->states = stack_create();
	if (head) group_push(head, t);
	frame_create(t, x, y, w, h);
	return t;
}
void group_destroy(group *t)
{
	while (t->frames)
		frame_destroy(t->frames);
	group_pop(t); free(t);
}
void group_unfocus(group *t)
{
	if (heads->groups == t) frame_unfocus(t->frames);
}
void group_hide(group *t)
{
	frame *f; client *c; int i;
	FOR_RING (f, t->frames, i)
		frame_hide(f);
	FOR_RING (c, t->clients, i)
		client_hide(c);
}
void group_focus(group *t)
{
	frame *f; int i;
	FOR_RING (f, t->frames, i)
		frame_update(f);
	frame_focus(t->frames);
}
void group_next()
{
	if (heads->groups != heads->groups->next)
	{
		group_unfocus(heads->groups);
		heads->groups = heads->groups->next;
		group_focus(heads->groups);
		// hide after to reduce flicker
		if (heads->groups->prev != heads->groups)
			group_hide(heads->groups->prev);
	}
}
void group_prev()
{
	if (heads->groups != heads->groups->prev)
	{
		group_unfocus(heads->groups);
		heads->groups = heads->groups->prev;
		group_focus(heads->groups);
		// hide after to reduce flicker
		if (heads->groups->next != heads->groups)
			group_hide(heads->groups->next);
	}
}
// resize frames to match changes in the screen border padding
void group_resize(group *ta, int l, int r, int t, int b)
{
	frame *f = ta->frames; int i;
	int sw = ta->head->screen->width;
	int sh = ta->head->screen->height;
	     if (l < ta->l) { frames_fill_gap(ta, l, ta->t, ta->l - l, sh - ta->b, FRAMES_ALL); ta->l = l; }
	else if (l > ta->l) { frames_make_gap(ta, ta->l, ta->t, l - ta->l, sh - ta->b); ta->l = l; }
	     if (r < ta->r) { frames_fill_gap(ta, sw - ta->r, ta->t, ta->r - r, sh - ta->b, FRAMES_ALL); ta->r = r; }
	else if (r > ta->r) { frames_make_gap(ta, sw - r, ta->t, r - ta->r, sh - ta->b); ta->r = r; }
	     if (t < ta->t) { frames_fill_gap(ta, ta->l, t, sw - ta->l - ta->r, ta->t - t, FRAMES_ALL); ta->t = t; }
	else if (t > ta->t) { frames_make_gap(ta, ta->l, ta->t, sw - ta->l - ta->r, t - ta->t); ta->t = t; }
	     if (b < ta->b) { frames_fill_gap(ta, ta->l, sh - ta->b, sw - ta->l - ta->r, ta->b - b, FRAMES_ALL); ta->b = b; }
	else if (b > ta->b) { frames_make_gap(ta, ta->l, sh - b, sw - ta->l - ta->r, b - ta->b); ta->b = b; }
	FOR_RING (f, ta->frames, i)
		frame_update(f);
}
group* group_by_name(head *h, char *name)
{
	group *t; int i;
	FOR_RING (t, h->groups, i)
		if (strcmp(t->name, name) == 0)
			return t;
	say("unknown group %s", name);
	return NULL;
}
void group_raise(char *name)
{
	group *t = group_by_name(heads, name);
	if (t && t != heads->groups)
	{
		group_unfocus(heads->groups);
		group *hide = heads->groups;
		heads->groups = t;
		group_focus(t);
		// hide after to reduce flicker
		if (hide != t) group_hide(hide);
	}
}
char* group_dump(group *g)
{
	frame *f; int i;
	autostr s; str_create(&s);
	str_require(&s, NOTE);
	s.len += sprintf(s.pad+s.len, "group\t%d\t%d\t%d\t%d\t%s\n", g->l, g->r, g->t, g->b, g->name);
	FOR_RING (f, g->frames, i)
	{
		str_require(&s, NOTE);
		s.len += sprintf(s.pad+s.len, "frame\t%d\t%d\t%d\t%d\t%u\t%s\n", f->x, f->y, f->w, f->h, f->flags,
			f->cli ? f->cli->class: NULL);
	}
	client *c;
	FOR_RING (c, g->clients, i)
	{
		str_require(&s, NOTE);
		s.len += sprintf(s.pad+s.len, "window\t%d\t%s\t%s\n", i, c->class, c->name);
	}
	return s.pad;
}
void group_load(group *g, char *dump)
{
	frame *f; client *c; int i;
	char *line = dump, *l = line, *r = l;
	int x, y, w, h; ubyte flags;
	while (g->frames) frame_destroy(g->frames);
	while (*line)
	{
		if (strncmp(line, "group", 5) == 0)
		{
			l = line + 5; strskip(&l, isspace);
			x = strtol(l, &l, 10); strskip(&l, isspace);
			y = strtol(l, &l, 10); strskip(&l, isspace);
			w = strtol(l, &l, 10); strskip(&l, isspace);
			h = strtol(l, &l, 10); strskip(&l, isspace);
			g->l = x; g->r = y; g->t = w; g->b = h;
			r = strchr(l, '\n');
			memmove(g->name, l, (r-l));
			g->name[r-l] = '\0';
		} else
		if (strncmp(line, "frame", 5) == 0)
		{
			l = line + 5; strskip(&l, isspace);
			x = strtol(l, &l, 10); strskip(&l, isspace);
			y = strtol(l, &l, 10); strskip(&l, isspace);
			w = strtol(l, &l, 10); strskip(&l, isspace);
			h = strtol(l, &l, 10); strskip(&l, isspace);
			flags = strtol(l, &l, 10); strskip(&l, isspace);
			f = frame_create(g, x, y, w, h); f->flags = flags;
			FOR_RING (c, g->clients, i)
			{
				if (!c->frame && strncmp(c->class, l, strlen(c->class)) == 0)
				{
					f->cli = c; c->frame = f;
					break;
				}
			}
		}
		strscanthese(&line, "\n");
		strskip(&line, isspace);
	}
	FOR_RING (f, g->frames, i) frame_update(f);
	frame_focus(g->frames);
}
void group_track(group *g)
{
	if (g->states->depth == STACK)
		free(stack_shift(g->states));
	stack_push(g->states, group_dump(g));
}
void group_undo()
{
	group *g = heads->groups;
	if (g->states->depth > 0)
		group_load(g, stack_pop(g->states));
	else	say("nothing to undo for %s", g->name);
}
void group_stack()
{
	group *g = heads->groups;
	if (g->flags & GF_STACKING)
	{
		g->flags &= ~GF_STACKING;
		g->flags |= GF_TILING;
		group_undo(g);
	} else
	{
		group_track(g);
		while (g->frames) frame_destroy(g->frames);
		frame_create(g, g->l, g->t,
			g->head->screen->width - g->l - g->r, g->head->screen->height - g->t - g->b);
		g->flags &= ~GF_TILING;
		g->flags |= GF_STACKING;
		client *c; int i;
		FOR_RING (c, g->clients, i)
		{
			if (c->fw > 0 && c->fh > 0)
			{
				c->x = c->fx; c->y = c->fy; c->w = c->fw; c->h = c->fh;
			} else
			{
				// compensate for now-invisible frame border
				c->x--; c->y--;
			}
			XMoveResizeWindow(display, c->win, c->x, c->y, c->w, c->h);
			c->fx = c->x; c->fy = c->y; c->fw = c->w; c->fh = c->h;
			client_display(c, g->frames);
		}
		frame_update(g->frames);
		frame_focus(g->frames);
	}
}

// HEADS

void head_focus(head *h)
{
	group_unfocus(heads->groups);
	heads = h;
	if (heads->display_string)
		putenv(heads->display_string);
	group_focus(heads->groups);
}
void head_next()
{
	head_focus(heads->next);
}

// EVENTS

void menu(char *cmd, char *after)
{
	char *tmp = allocate(strlen(cmd)+strlen(after)+BLOCK);
	sprintf(tmp, "%s | %s | %s", cmd, settings[ms_dmenu].s, after);
	exec_cmd(tmp); free(tmp);
}
void shutdown()
{
	// we just bail out for now.  any clients that want to stay alive, and are
	// not our descendants, can do so.
	// this assumes that the X session will not also stop when we die :-)
	exit(EXIT_SUCCESS);
}
dcell parse_size(char *cmd, regmatch_t *subs, ucell index, ucell limit)
{
	dcell size = 0, an = 0, bn = 0;
	char *a = regsubstr(cmd, subs, index);
	char *b = regsubstr(cmd, subs, index+1);
	an = strtod(a, NULL);
	if (b) bn = strtod(b, NULL);
	if (strchr(a, '%')) size = (an / 100) * limit; // percent
	else if (!b) size = an; // pixels
	else	size = (an / bn) * limit; // fraction
	free(a); free(b);
	return MIN(MAX(size, settings[ms_frame_min_wh].u), limit);
}
void com_frame_split(char *cmd, regmatch_t *subs)
{
	group *g = heads->groups; frame *f = g->frames;
	int sw = heads->screen->width  - g->l - g->r,
	    sh = heads->screen->height - g->t - g->b;
	char *mode = regsubstr(cmd, subs, 1);
	ucell fs = f->h, ss = sh; ubyte dir = VERTICAL;
	if (*mode == 'h') { fs = f->w; ss = sw; dir = HORIZONTAL; }
	dcell size = parse_size(cmd, subs, 2, fs);
	frame_split(dir, size / fs);
	free(mode);
}
void com_frame_size(char *cmd, regmatch_t *subs)
{
	group *g = heads->groups; frame *f = g->frames;
	int sw = heads->screen->width  - g->l - g->r,
	    sh = heads->screen->height - g->t - g->b;
	char *mode = regsubstr(cmd, subs, 1);
	ucell fs = f->h, ss = sh; ubyte dir = VERTICAL;
	if (*mode == 'w') { fs = f->w; ss = sw; dir = HORIZONTAL; }
	dcell size = parse_size(cmd, subs, 2, ss);
	// auto-create another frame if we are full w/h
	if (fs == ss)
		frame_split(dir, size / ss);
	else {
		if (fs > size)
			frame_shrink(dir, 0, fs - size);
		else	frame_grow(dir, 0, size - fs);
	}
	free(mode);
}
void com_frame_resize(char *cmd, regmatch_t *subs)
{
	char *op = regsubstr(cmd, subs, 1);
	     if (*op == 'u') frame_shrink(VERTICAL, 1, settings[ms_frame_resize].u);
	else if (*op == 'd') frame_grow(VERTICAL, 1, settings[ms_frame_resize].u);
	else if (*op == 'l') frame_shrink(HORIZONTAL, 1, settings[ms_frame_resize].u);
	else if (*op == 'r') frame_grow(HORIZONTAL, 1, settings[ms_frame_resize].u);
	free(op);
}
void com_group_pad(char *cmd, regmatch_t *subs)
{
	char *l = regsubstr(cmd, subs, 1);
	char *r = regsubstr(cmd, subs, 2);
	char *t = regsubstr(cmd, subs, 3);
	char *b = regsubstr(cmd, subs, 4);
	int ln = strtol(l, NULL, 10);
	int rn = strtol(r, NULL, 10);
	int tn = strtol(t, NULL, 10);
	int bn = strtol(b, NULL, 10);
	group_resize(heads->groups, ln, rn, tn, bn);
	free(l); free(r); free(t); free(b);
}
void com_group_add(char *cmd, regmatch_t *subs)
{
	char *name = regsubstr(cmd, subs, 1);
	group_create(heads, name, 0, 0, heads->screen->width, heads->screen->height);
	group_prev(); free(name);
}
void com_group_drop(char *cmd, regmatch_t *subs)
{
	group *active = heads->groups;
	char *name = regsubstr(cmd, subs, 1);
	group *t = group_by_name(heads, name);
	if (t && heads->groups != heads->groups->next)
	{
		group_destroy(t);
		if (t == active) group_focus(heads->groups);
	}
	free(name);
}
void com_group_name(char *cmd, regmatch_t *subs)
{
	char *name = regsubstr(cmd, subs, 1);
	group *t = heads->groups;
	snprintf(t->name, 32, "%s", name);
	free(name);
}
void com_window_to_group(char *cmd, regmatch_t *subs)
{
	char *name = regsubstr(cmd, subs, 1);
	client *c = heads->groups->frames->cli;
	while (c->parent && is_valid_client(c->parent))
		c = c->parent;
	group *g = group_by_name(heads, name);
	if (c && g)
	{
		client_regroup(g, c);
		frame_focus_hidden(heads->groups->frames);
	}
	free(name);
}
void com_window_raise(char *cmd, regmatch_t *subs)
{
	client *c, *m = NULL; int i;
	char *slot = regsubstr(cmd, subs, 1);
	if (isdigit(*slot))
	{
		ucell num = strtol(slot, NULL, 10);
		FOR_RING (c, heads->groups->clients, i)
		{
			if (i == num)
			{
				m = c;
				break;
			}
		}
	} else
	{
		FOR_RING (c, heads->groups->clients, i)
		{
			if (strcmp(slot, c->name) == 0)
			{
				m = c;
				break;
			}
		}
	}
	if (m)
	{
		if (m->frame && m->frame->cli == m)
			frame_focus(m->frame);
		else client_focus(m, heads->groups->frames);
	}
	free(slot);
}
void com_frame_swap(char *cmd, regmatch_t *subs)
{
	char *op = regsubstr(cmd, subs, 1);
	     if (*op == 'u') frame_swap(TOP);
	else if (*op == 'd') frame_swap(BOTTOM);
	else if (*op == 'l') frame_swap(LEFT);
	else if (*op == 'r') frame_swap(RIGHT);
	free(op);
}
void com_group_dump(char *cmd, regmatch_t *subs)
{
	char *file = regsubstr(cmd, subs, 1);
	group *g = heads->groups;
	if (g)
	{
		char *dump = group_dump(g);
		try (oops) {
			blurt(file, dump);
		} catch (oops);
		if (oops.code)
			say("could not write %s", file);
		if (dump) free(dump);
	}
	free(file);
}
void com_group_load(char *cmd, regmatch_t *subs)
{
	char *file = regsubstr(cmd, subs, 1);
	group *g = heads->groups;
	if (g)
	{
		char *dump = NULL;
		try (oops) {
			dump = slurp(file, NULL);
			group_track(g);
			group_load(g, dump);
		} catch (oops);
		if (oops.code)
			say("could not read %s", file);
		if (dump) free(dump);
	}
	free(file);
}
void com_frame_remove(char *cmd, regmatch_t *subs)
{
	frame_remove();
}
void com_frame_kill(char *cmd, regmatch_t *subs)
{
	frame_kill_client();
}
void com_frame_cycle(char *cmd, regmatch_t *subs)
{
	frame_cycle_client();
}
void com_frame_only(char *cmd, regmatch_t *subs)
{
	frame_only();
}
void com_group_undo(char *cmd, regmatch_t *subs)
{
	group_undo();
}
ubyte parse_flag(char *cmd, regmatch_t *subs, ucell index, ubyte current)
{
	char *mode = regsubstr(cmd, subs, index);
	ubyte flag = 0;
	if (strcmp(mode, "on") == 0) flag = 1;
	if (strcmp(mode, "flip") == 0)
		flag = current ? 0: 1;
	free(mode);
	return flag;
}
void com_frame_dedicate(char *cmd, regmatch_t *subs)
{
	frame *f = heads->groups->frames;
	ubyte on = parse_flag(cmd, subs, 1, f->flags & FF_DEDICATE);
	if ((on && !(f->flags & FF_DEDICATE)) ||
		(!on && f->flags & FF_DEDICATE))
		frame_dedicate();
}
void com_frame_catchall(char *cmd, regmatch_t *subs)
{
	frame *f = heads->groups->frames;
	ubyte on = parse_flag(cmd, subs, 1, f->flags & FF_CATCHALL);
	if ((on && !(f->flags & FF_CATCHALL)) ||
		(!on && f->flags & FF_CATCHALL))
		frame_catchall();
}
void com_frame_focus(char *cmd, regmatch_t *subs)
{
	char *dir = regsubstr(cmd, subs, 1);
		if (*dir == 'u') frame_up();
	else	if (*dir == 'd') frame_down();
	else	if (*dir == 'l') frame_left();
	else	if (*dir == 'r') frame_right();
	free(dir);
}
void com_group_use(char *cmd, regmatch_t *subs)
{
	char *name = regsubstr(cmd, subs, 1);
	if (strcmp(name, "(next)") == 0) group_next();
	else if (strcmp(name, "(prev)") == 0) group_prev();
	else group_raise(name);
	free(name);
}
void com_exec(char *cmd, regmatch_t *subs)
{
	char *exec = regsubstr(cmd, subs, 1);
	if (strlen(exec)) exec_cmd(exec);
	free(exec);
}
void com_manage(char *cmd, regmatch_t *subs)
{
	ubyte on = parse_flag(cmd, subs, 1, 0);
	char *name = regsubstr(cmd, subs, 2);
	if (!on && !is_unmanaged_class(name))
		stack_push(unmanaged, name);
	else
	if (on)
	{
		int i; for (i = 0; i < unmanaged->depth; i++)
			if (strcmp((char*)(unmanaged->items[i]), name) == 0)
				stack_del(unmanaged, i);
	}
	say("%s is %s", name, on ? "managed": "unmanaged");
}
void com_screen_switch(char *cmd, regmatch_t *subs)
{
	char *num = regsubstr(cmd, subs, 1);
	if (strcmp(num, "(next)") == 0) head_next();
	else
	{
		ucell n = strtol(num, NULL, 10);
		head *h; int i; ubyte found = 0;
		FOR_RING (h, heads, i)
		{
			if (h->id == n)
			{
				head_focus(h);
				found = 1;
				break;
			}
		}
		if (!found)
			say("invalid screen id %d", n);
	}
	free(num);
}
void com_group_stack(char *cmd, regmatch_t *subs)
{
	group *g = heads->groups;
	ubyte on = parse_flag(cmd, subs, 1, g->flags & GF_STACKING);
	if ((on && !(g->flags & GF_STACKING)) ||
		(!on && g->flags & GF_STACKING))
		group_stack();
}
void com_set(char *cmd, regmatch_t *subs)
{
	int i;
	char *name = regsubstr(cmd, subs, 1);
	char *value = regsubstr(cmd, subs, 2);
	for (i = 0; i < ms_last; i++)
	{
		if (strcmp(settings[i].name, name) == 0)
		{
			switch (settings[i].type)
			{
				case mst_str:
					free(settings[i].s);
					settings[i].s = value;
					say("set %s to %s", name, value);
					break;
				case mst_ucell:
					settings[i].u = strtol(value, NULL, 10);
					say("set %s to %u", name, settings[i].u);
					free(value);
					break;
				case mst_dcell:
					settings[i].d = strtod(value, NULL);
					say("set %s to %f", name, settings[i].d);
					free(value);
					break;
			}
			free(name);
			return;
		}
	}
	say("invalid setting name: %s", name);
	free(name); free(value);
}
void com_bind(char *cmd, regmatch_t *subs)
{
	ubyte flag = parse_flag(cmd, subs, 1, 0);
	char *pattern = regsubstr(cmd, subs, 2);
	if (flag)
	{
		char *command = regsubstr(cmd, subs, 3);
		if (command)
		{
			strtrim(command);
			if (create_binding(pattern, command))
				say("bound %s to %s", pattern, command);
			else	say("could not bind %s", pattern);
			free(command);
		}
	} else
	{
		if (remove_binding(pattern))
			say("unbound %s", pattern);
		else	say("could not unbind %s", pattern);
	}
	grab_stuff();
	free(pattern);
}
void com_switch(char *cmd, regmatch_t *subs)
{
	char *mode = regsubstr(cmd, subs, 1);
	if (strcmp(mode, "window") == 0) window_switch();
	else if (strcmp(mode, "group") == 0) group_switch();
	free(mode);
}
void com_command(char *cmd, regmatch_t *subs)
{
	int i;
	autostr s; str_create(&s);
	str_append(&s, "echo \"", 6);
	for (i = 0; i < (sizeof(commands)/sizeof(char*)); i++)
	{
		str_require(&s, NOTE);
		s.len += sprintf(s.pad+s.len, "%s \n", commands[i]);
	}
	str_drop(&s, 1); str_append(&s, "\"", 1);
	menu(s.pad, settings[ms_run_musca_command].s);
	str_delete(&s);
}
void com_shell(char *cmd, regmatch_t *subs)
{
	menu("dmenu_path", settings[ms_run_shell_command].s);
}
void com_quit(char *cmd, regmatch_t *subs)
{
	shutdown();
}
void musca_command(char *cmd)
{
	group *g = heads->groups;
	regmatch_t subs[10];
	int i, matches = 0, len = strlen(cmd);
	if (len)
	{
		for (i = 0; i < (sizeof(command_callbacks)/sizeof(struct command_map)); i++)
		{
			// todo: precompile these during setup()
			if (regmatch(NULL, command_callbacks[i].pattern, cmd, 10, subs, REG_EXTENDED|REG_ICASE))
			{
				matches++;
				if (command_callbacks[i].flags & g->flags)
					(command_callbacks[i].func)(cmd, subs);
				else say("command invalid for %s mode: %s", g->flags & GF_TILING ? "tiling": "stacking", cmd);
			}
			if (matches) break;
		}
	}
	if (!matches && strlen(cmd))
		say("unknown command: %s", cmd);
}
void catch_exit(int sig)
{
	while (0 < waitpid(-1, NULL, WNOHANG));
}

void exec_cmd(char *cmd)
{
	signal(SIGCHLD, catch_exit);
	if (fork() == 0)
	{
		if (heads->display_string)
			putenv(heads->display_string);
		setsid();
		execlp("/bin/sh", "sh", "-c", cmd, NULL);
		exit(EXIT_FAILURE);
	}
}
void window_switch()
{
	group *g = heads->groups;
	autostr names; str_create(&names);
	str_append(&names, "echo \"", 6);
	client *c; int num = 0; char pad[NOTE];
	FOR_RING (c, g->clients, num)
	{
		sprintf(pad, "%d ", num);
		str_append(&names, pad, strlen(pad));
		str_append(&names, c->name, strlen(c->name));
		str_append(&names, " \n", 2);
	}
	if (names.len) str_drop(&names, 1);
	str_append(&names, "\"", 1);
	menu(names.pad, settings[ms_switch_window].s);
	str_delete(&names);
}
void group_switch()
{
	head *h = heads;
	autostr names; str_create(&names);
	str_append(&names, "echo \"", 6);
	group *g; int i;
	FOR_RING (g, h->groups, i)
	{
		str_append(&names, g->name, strlen(g->name));
		str_append(&names, " \n", 2);
	}
	if (names.len) str_drop(&names, 1);
	str_append(&names, "\"", 1);
	menu(names.pad, settings[ms_switch_group].s);
	str_delete(&names);
}
void manage(Window win, XWindowAttributes *attr)
{
	client *c, *p;
	head *h = head_by_screen(attr->screen);
	Window trans = None;
	// transients use parent's frame even if it is dedicated
	if (XGetTransientForHint(display, win, &trans) && (p = client_by_window(trans)))
	{
		c = client_create(h->groups, h->groups->frames, win);
		c->parent = p; p->kids++;
	} else
	{
		c = client_create(h->groups, frame_available(h->groups->frames), win);
	}
	client_focus(c, c->frame);
	frames_display_hidden(c->group);
}
winstate* quiz_window(Window win)
{
	winstate *state = allocate(sizeof(winstate));
	state->w = win; state->c = NULL; state->f = NULL; state->manage = 0;
	if ((state->ok = XGetWindowAttributes(display, win, &state->attr)))
	{
		state->c = client_by_window(win);
		if (!state->c) state->f = is_frame_background(win);
		state->manage = !state->c && !state->f && !is_unmanaged_window(win) ? 1:0;
	}
	return state;
}
void createnotify(XEvent *ev)
{
	winstate *ws = quiz_window(ev->xcreatewindow.window);
	if (ws->ok && !ws->attr.override_redirect)
		XSelectInput(display, ws->w, PropertyChangeMask);
	free(ws);
}
void configurerequest(XEvent *ev)
{
	XConfigureRequestEvent *cr = &ev->xconfigurerequest;
	winstate *ws = quiz_window(cr->window);
	if (ws->ok && !ws->attr.override_redirect)
	{
		if (ws->c)
			client_configure(ws->c, cr);
		else
		{
			XWindowChanges wc;
			wc.x = cr->x; wc.y = cr->y;
			wc.width = MAX(1, cr->width);
			wc.height = MAX(1, cr->height);
			wc.border_width = cr->border_width;
			wc.sibling = cr->above;
			wc.stack_mode = cr->detail;
			XConfigureWindow(display, cr->window, cr->value_mask, &wc);
		}
	}
	free(ws);
}
void configurenotify(XEvent *ev)
{
}
void maprequest(XEvent *ev)
{
	winstate *ws = quiz_window(ev->xmaprequest.window);
	if (ws->ok && !ws->attr.override_redirect)
	{
		if (ws->manage && heads->groups->flags & GF_TILING)
			// some clients seem unwilling to be sized for tiling at the maprequest stage
			// (mplayer is one). so, we let them map off screen, and begin actual management
			// in mapnotify.
			XMoveResizeWindow(display, ws->w,
				heads->screen->width, heads->screen->height, ws->attr.width, ws->attr.height);
		XMapWindow(display, ws->w);
	}
	free(ws);
}
void mapnotify(XEvent *ev)
{
	winstate *ws = quiz_window(ev->xmap.window);
	if (ws->ok && !ws->attr.override_redirect)
	{
		if (ws->manage) manage(ws->w, &ws->attr);
		else if (ws->c) client_display(ws->c, NULL);
	}
	free(ws);
}
void unmapnotify(XEvent *ev)
{
	winstate *ws = quiz_window(ev->xunmap.window);
	if (ws->ok && !ws->attr.override_redirect)
	{
		// did we *not* tell it to unmap during group switch?  if so, this is a voluntary
		// unmap (perhaps minimizing to a sys tray or similar) so wish it bon voyage and forget.
		if (ws->c && !(ws->c->flags & CF_HIDDEN))
			client_remove(ws->c);
	}
	free(ws);
}
void mappingnotify(XEvent *ev)
{
	XMappingEvent *me = &ev->xmapping;
	if (me->request == MappingKeyboard || me->request == MappingModifier)
	{
		XRefreshKeyboardMapping(me);
		grab_stuff();
	}
}
void destroynotify(XEvent *ev)
{
	Window win = ev->xdestroywindow.window;
	client *c = client_by_window(win);
	if (c) client_remove(c);
}
void keypress(XEvent *ev)
{
	XKeyEvent *key = &ev->xkey;
	struct binding *kb = find_binding(key->state & ~(NumlockMask | LockMask), key->keycode);
	if (kb) musca_command(kb->command);
	else note("unknown XKeyEvent %u %d", key->state, key->keycode);
}
void buttonpress(XEvent *ev)
{
	// *any* mouse button press, including rolling the wheel, arrives here and will cause
	// a frame and client focus change
	XButtonEvent *button = &ev->xbutton; frame *f;
	if (button->subwindow != None)
	{
		client *c = NULL;
		if (!(f = is_frame_background(button->subwindow)))
		{
			c = client_by_window(button->subwindow);
			if (c) f = c->frame;
		}
		if (f)
		{
			if (f->group->head != heads)   head_focus(f->group->head);
			if (f->group != heads->groups) group_focus(f->group);
			if (f->group->frames != f)     frame_focus(f);
			// start drag for resize or move in stack mode
			if (c && c->group->flags & GF_STACKING && c)
			{
				client_focus(c, f);

				if (button->state & modifier_names_to_mask(settings[ms_stack_mouse_modifier].s))
				{
					c->rt = window_create(heads->screen->root, c->x, c->y, c->w+1, 1, 0, NULL, settings[ms_border_focus].s, "musca", "musca");
					c->rl = window_create(heads->screen->root, c->x, c->y, 1, c->h+1, 0, NULL, settings[ms_border_focus].s, "musca", "musca");
					c->rr = window_create(heads->screen->root, c->x + c->w+1, c->y, 1, c->h+2, 0, NULL, settings[ms_border_focus].s, "musca", "musca");
					c->rb = window_create(heads->screen->root, c->x, c->y + c->h+1, c->w+2, 1, 0, NULL, settings[ms_border_focus].s, "musca", "musca");
					XMapWindow(display, c->rl); XMapWindow(display, c->rt); XMapWindow(display, c->rr); XMapWindow(display, c->rb);
					XRaiseWindow(display, c->rl); XRaiseWindow(display, c->rt); XRaiseWindow(display, c->rr); XRaiseWindow(display, c->rb);

					XGrabPointer(display, c->rl, True, PointerMotionMask|ButtonReleaseMask,
						GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
					c->mxr = button->x_root; c->myr = button->y_root; c->mb = button->button;
					c->mx = c->x; c->my = c->y; c->mw = c->w; c->mh = c->h;
				}
			}
		}
	}
	XAllowEvents(display, ReplayPointer, CurrentTime);
}
void buttonrelease(XEvent *ev)
{
	XButtonEvent *button = &ev->xbutton;
	client *c = client_by_resize_window(button->window);
	if (c)
	{
		XMoveResizeWindow(display, c->win, c->x, c->y, c->w, c->h);
		XUngrabPointer(display, CurrentTime);
		XDestroyWindow(display, c->rl); XDestroyWindow(display, c->rt); XDestroyWindow(display, c->rr); XDestroyWindow(display, c->rb);
		c->rl = 0; c->rr = 0; c->rt = 0; c->rb = 0;
		client_focus(c, NULL);
	}
}
void motionnotify(XEvent *ev)
{
	// we should only get these if we've grabbed the pointer
	while (XCheckTypedEvent(display, MotionNotify, ev));
	XMotionEvent *motion = &ev->xmotion;
	XButtonEvent *button = &ev->xbutton;
	if (motion->same_screen)
	{
		client *c = client_by_resize_window(motion->window);
		if (c)
		{
			int dx = MAX(c->x, button->x_root) - c->mxr, dy = MAX(c->y, button->y_root) - c->myr;
			c->x = c->mx + (c->mb == Button1 ? dx: 0); c->y = c->my + (c->mb == Button1 ? dy: 0);
			// should limit maximum size?
			c->w = MAX(settings[ms_frame_min_wh].u, c->mw + (c->mb == Button3 ? dx : 0));
			c->h = MAX(settings[ms_frame_min_wh].u, c->mh + (c->mb == Button3 ? dy : 0));
			XMoveResizeWindow(display, c->rt, c->x, c->y, c->w+1, 1);
			XMoveResizeWindow(display, c->rl, c->x, c->y, 1, c->h+1);
			XMoveResizeWindow(display, c->rr, c->x + c->w+1, c->y, 1, c->h+2);
			XMoveResizeWindow(display, c->rb, c->x, c->y + c->h+1, c->w+2, 1);
			c->fx = c->x; c->fy = c->y; c->fw = c->w; c->fh = c->h;
		}
	}
}
void propertynotify(XEvent *ev)
{
	XPropertyEvent *pe = &ev->xproperty;
	if (pe->atom == XA_WM_NAME || pe->atom == netatom[NetWMName])
	{
		client *c = client_by_window(pe->window);
		if (c) window_name(c->win, c->name);
	} else
	if (pe->atom == muscatom[MuscaCommand])
	{
		char cmd[BLOCK]; window_property(pe->window, muscatom[MuscaCommand], cmd, BLOCK);
		printf("received: %s\n", cmd);
		if (strtrim(cmd)) musca_command(cmd);
		XChangeProperty (display, pe->window, muscatom[MuscaResult],
			XA_STRING, 8, PropModeReplace, (ubyte*)"0", 2);
	}
}
// thanks to ratpoison for the -c command communications idea and basic mechanism
ubyte insert_command(char* cmd)
{
	Window win = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, None, None);
	XSelectInput (display, win, PropertyChangeMask);
	XClassHint *hint = XAllocClassHint(); hint->res_name = "musca"; hint->res_class = "musca";
	XSetClassHint(display, win, hint);
	XSync(display, False);
	XChangeProperty (display, win, muscatom[MuscaCommand],
		XA_STRING, 8, PropModeReplace, (ubyte*)cmd, strlen(cmd) + 1);
	ubyte ok = 0;
	for (;;)
	{
		XEvent ev;
		XMaskEvent (display, PropertyChangeMask, &ev);
		if (ev.xproperty.atom == muscatom[MuscaResult]
	          && ev.xproperty.state == PropertyNewValue)
		{
			char result[NOTE];
			window_property(win, muscatom[MuscaResult], result, NOTE);
			ok = strtol(result, NULL, 10);
			break;
		}
		usleep(10000);
	}
	XFree(hint);
	XDestroyWindow(display, win);
	return ok;
}
void find_clients(head *h)
{
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa; ucell num; int i;
	// find any unmanaged running clients and manage them
	if (XQueryTree(display, h->screen->root, &d1, &d2, &wins, &num))
	{
		// normals first
		for (i = 0; i < num; i++)
		{
			if (!XGetWindowAttributes(display, wins[i], &wa)
				|| wa.override_redirect
				|| XGetTransientForHint(display, wins[i], &d1)
				|| is_frame_background(wins[i])
				|| wa.map_state != IsViewable
				|| is_unmanaged_window(wins[i])
				|| client_by_window(wins[i])
				)
				continue;
			manage(wins[i], &wa);
		}
		// transients second and on top
		for(i = 0; i < num; i++)
		{
			if (!XGetWindowAttributes(display, wins[i], &wa)
				|| wa.override_redirect
				|| is_frame_background(wins[i])
				|| wa.map_state != IsViewable
				|| is_unmanaged_window(wins[i])
				|| client_by_window(wins[i])
				)
				continue;
			if (XGetTransientForHint(display, wins[i], &d1))
				manage(wins[i], &wa);
		}
		if(wins) XFree(wins);
	}
}
void ungrab_stuff()
{
	head *h; int i;
	FOR_RING (h, heads, i)
	{
		XUngrabKey(display, AnyKey, AnyModifier, h->screen->root);
		XUngrabButton(display, AnyButton, AnyModifier, h->screen->root);
	}
}
void grab_stuff()
{
	int i, j; head *h;
	ungrab_stuff();
	ucell modifiers[] = { 0, LockMask, NumlockMask, LockMask|NumlockMask };
	FOR_RING (h, heads, i)
	{
		for (i = 0; i < bindings->depth; i++)
		{
			struct binding *kb = bindings->items[i];
			for (j = 0; j < (sizeof(modifiers)/sizeof(ucell)); j++)
				XGrabKey(display, kb->key, kb->mod | modifiers[j], h->screen->root,
					True, GrabModeAsync, GrabModeAsync);
		}
		XGrabButton(display, AnyButton, AnyModifier, h->screen->root,
			True, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
	}
}
void setup(int argc, char **argv)
{
	int i, j; head *f, *p, *h; heads = NULL;
	xerrorxlib = XSetErrorHandler(error_callback);
	assert((display = XOpenDisplay(0x0)), "cannot open display");
	// some framework
	self = allocate(NOTE); sprintf(self, "MUSCA=%s", argv[0]);
	assert(putenv(self) == 0, "cannot set $MUSCA environment variable");
	for (i = 0; i < ms_last; i++)
	{
		if (settings[i].type == mst_str)
		{
			char *tmp = allocate(strlen(settings[i].s)+1);
			strcpy(tmp, settings[i].s); settings[i].s = tmp;
		}
	}
	// thanks to dwm for this bare minimum subset of atoms we care about
	wmatom[WMProtocols]   = XInternAtom(display, "WM_PROTOCOLS",     False);
	wmatom[WMDelete]      = XInternAtom(display, "WM_DELETE_WINDOW", False);
	wmatom[WMState]       = XInternAtom(display, "WM_STATE",         False);
	netatom[NetWMName]    = XInternAtom(display, "_NET_WM_NAME",     False);
	// some more
	wmatom[WMClass]        = XInternAtom(display, "WM_CLASS",         False);
	wmatom[WMName]         = XInternAtom(display, "WM_NAME",          False);
	muscatom[MuscaCommand] = XInternAtom(display, "MUSCA_COMMAND",    False);
	muscatom[MuscaResult]  = XInternAtom(display, "MUSCA_RESULT",     False);
	// process args
	char o;
	while ((o = getopt(argc, argv, "c:")) != -1)
	{
		switch (o)
		{
			case 'c':
				exit(insert_command(optarg) ? EXIT_FAILURE: EXIT_SUCCESS);
				break;
		}
	}
	// init NumlockMask (thanks to Nikita Kanounnikov)
	XModifierKeymap *modmap;
	modmap = XGetModifierMapping(display);
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < modmap->max_keypermod; j++)
		{
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(display, XK_Num_Lock))
			{
				NumlockMask = (1 << i);
				break;
			}
		}
	}
	XFreeModifiermap(modmap);
	// prepare key bindings
	bindings = stack_create();
	for (i = 0; i < (sizeof(keymaps)/sizeof(struct keymap)); i++)
		create_binding(keymaps[i].pattern, keymaps[i].command);
	// initialize unmanaged stack from unmanaged_windows[]
	unmanaged = stack_create();
	for (i = 0; i < (sizeof(unmanaged_windows)/sizeof(char*)); i++)
	{
		char *cpy = allocate(strlen(unmanaged_windows[i])+1);
		strcpy(cpy, unmanaged_windows[i]);
		stack_push(unmanaged, cpy);
	}
	// intialize all heads
	f = NULL; p = NULL;
	for (i = 0; i < ScreenCount(display); i++)
	{
		h = allocate(sizeof(head)); h->id = i;
		h->screen = XScreenOfDisplay(display, i);
		h->display_string = NULL;
		h->prev = p; h->next = f;
		h->groups = NULL;

		if (!heads && DefaultScreen(display) == i) heads = h;
		group_create(h, "default", 0, 0, h->screen->width, h->screen->height);

		if (p) p->next = h;
		p = h; if (!f) f = h;

		XSelectInput(display, h->screen->root, SubstructureRedirectMask | SubstructureNotifyMask
			| StructureNotifyMask | FocusChangeMask);
		XDefineCursor(display, h->screen->root, XCreateFontCursor(display, XC_left_ptr));
	}
	f->prev = p; p->next = f;
	// none of below can be moved above as it requires the head linked list ring to be intact,
	// which it isn't until just now :-)
	h = heads;
	do {
		group_focus(heads->groups);
		char *ds = allocate(NOTE);
		sprintf(ds, "DISPLAY=%s", DisplayString(display));
		if (strrchr(DisplayString(display), ':'))
		{
			char *dot = strrchr(ds, '.');
			if (dot) sprintf(dot, ".%i", heads->id);
		}
		heads->display_string = ds;
		find_clients(heads);
		head_next();
	} while (heads != h);
	group_focus(heads->groups);
	// process startup file
	char *startup = NULL;
	try (oops) startup = slurp(settings[ms_startup].s, NULL); catch(oops);
	if (!oops.code && startup)
	{
		say("found %s", settings[ms_startup].s);
		char *l = startup, *r, eol;
		while (*l)
		{
			strskip(&l, isspace); r = l;
			strscanthese(&r, "\n"); eol = *r; *r = '\0';
			if (*l != '#') if (strtrim(l)) musca_command(l);
			*r = eol; l = r; strskip(&l, isspace);
		}
	}
	free(startup);
	FOR_RING (h, heads, i)
	{
		if (XGrabKeyboard(display, h->screen->root, False, GrabModeSync, GrabModeSync, CurrentTime) != 0)
			note("Could not temporarily grab keyboard. Something might be blocking key strokes.");
		XUngrabKeyboard(display, CurrentTime);
	}
	grab_stuff();
}
void process_event(XEvent *ev)
{
	try (event)
	{
		switch (ev->type)
		{
			case MapRequest:
				maprequest(ev);
				break;
			case DestroyNotify:
				destroynotify(ev);
				break;
			case ConfigureRequest:
				configurerequest(ev);
				break;
			case KeyPress:
				keypress(ev);
				break;
			case ButtonPress:
				buttonpress(ev);
				break;
			case ButtonRelease:
				buttonrelease(ev);
				break;
			case PropertyNotify:
				propertynotify(ev);
				break;
			case MapNotify:
				mapnotify(ev);
				break;
			case FocusIn:
				break;
			case FocusOut:
				break;
			case CreateNotify:
				createnotify(ev);
				break;
			case ConfigureNotify:
				configurenotify(ev);
				break;
			case UnmapNotify:
				unmapnotify(ev);
				break;
			case KeyRelease:
				break;
			case MotionNotify:
				motionnotify(ev);
				break;
			case MappingNotify:
				mappingnotify(ev);
				break;
			default:
				note("unhandled event %d", ev->type);
				break;
		}
	}
	// if we catch something, it is our own throw which we've planned for, or a
	// recoverable error let through by our installed x error handler.  we can
	// hopefully continue.
	catch (event);
}
int main(int argc, char **argv)
{
	XEvent ev;
	try (oops)
	{
		setup(argc, argv);
		for(;;)
		{
			XNextEvent(display, &ev);
			process_event(&ev);
		}
	} catch(oops);
	exit(oops.code ? EXIT_FAILURE: EXIT_SUCCESS);
}
