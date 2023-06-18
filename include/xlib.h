#ifndef XLIB_H
#define XLIB_H


#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <log.h>


/* types */
typedef struct{
	XID xid;

	int top,
		left,
		bottom,
		right;

	bool connected;
} xlib_monitor_t;

typedef struct xlib_win_t{
	Window id;
	int desktop,
		monitor;

	char name[128];
	int top,
		left,
		bottom,
		right;
	unsigned int width,
				 height;
	bool visible;

	unsigned int nchilds;
	struct xlib_win_t *childs;
} xlib_win_t;

typedef struct{
	int screen;
	Display *dpy;
	Window root;

	int desktop;
	xlib_monitor_t *monitors;
	size_t nmonitors;
	Window focus;
} xlib_t;


/* prototypes */
int xlib_init(xlib_t *xobj, char const *display);
void xlib_destroy(xlib_t *xobj);

void xlib_sync(xlib_t *xobj);

int xlib_win_init(xlib_t *xobj, Window id, xlib_win_t *win);
void xlib_win_destroy(xlib_win_t *win);
int xlib_win_childs(xlib_t *xobj, xlib_win_t *win);
void xlib_win_match_monitor(xlib_t *xobj, xlib_win_t *win);
void xlib_win_info(xlib_win_t *win, log_lvl_t lvl, char const *indent);

int xlib_win_focus(xlib_t *xobj, xlib_win_t *win);
int xlib_win_summon(xlib_t *xobj, xlib_win_t *win, int desktop, int x, int y);
int xlib_win_move(xlib_t *xobj, xlib_win_t *win, int x, int y, bool relative);
int xlib_win_map(xlib_t *xobj, xlib_win_t *win);
int xlib_win_unmap(xlib_t *xobj, xlib_win_t *win);

int xlib_mouse_moveto(xlib_t *xobj, int x, int y);


#endif // XLIB_H
