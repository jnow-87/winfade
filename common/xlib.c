#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <log.h>
#include <xlib.h>


/* local/static prototypes */
static int error_handler(Display *dsp, XErrorEvent *evt);
static int monitors_init(xlib_t *xobj);
static int win_read_prop(xlib_t *xobj, Window win, char const *name, int el_size, size_t max_el, void *prop);


/* global functions */
int xlib_init(xlib_t *xobj, char const *display){
	int dummy;


	XSetErrorHandler(error_handler);
	xobj->dpy = XOpenDisplay(display);

	if(xobj->dpy == 0x0)
		goto_err(err_0, "opening display\n");

	xobj->screen = XDefaultScreen(xobj->dpy);
	xobj->root = XDefaultRootWindow(xobj->dpy);

	if(monitors_init(xobj) != 0)
		goto err_1;

	if(win_read_prop(xobj, xobj->root, "_NET_CURRENT_DESKTOP", sizeof(int), 1, &xobj->desktop) != 0)
		goto_err(err_1, "reading current desktop\n");

	if(XGetInputFocus(xobj->dpy, &xobj->focus, &dummy) != 1)
		goto_err(err_1, "reading focused window\n");

	return 0;


err_1:
	xlib_destroy(xobj);

err_0:
	return ERROR("initialsing xlib\n");
}

void xlib_destroy(xlib_t *xobj){
	free(xobj->monitors);
	XCloseDisplay(xobj->dpy);
}

void xlib_sync(xlib_t *xobj){
	XSync(xobj->dpy, false);
}

int xlib_win_init(xlib_t *xobj, Window id, xlib_win_t *win){
	unsigned long decore[4];
	XWindowAttributes attrs;
	Window dummy;


	win->id = id;
	win->monitor = 0;
	win->desktop = -1;
	win->nchilds = 0;
	win->childs = 0x0;

	if(XGetWindowAttributes(xobj->dpy, id, &attrs) == 0)
		return -1;

	/* position and geometry */
	XTranslateCoordinates(xobj->dpy, id, xobj->root, 0, 0, &win->left, &win->top, &dummy);

	win->width = attrs.width;
	win->height = attrs.height;

	// remove frame extends
	// 	decore[0] - border width (left)
	// 	decore[1] - border width (right)
	// 	decore[2] - title bar height + 2 * border width
	// 	decore[3] - handle height + border width
	if(win_read_prop(xobj, win->id, "_NET_FRAME_EXTENTS", sizeof(long), 4, decore) == 0){
		win->left -= decore[0];
		win->top -= decore[2] * 2 - decore[0];	// having a factor of 2 probably only works for fluxbox
		win->width += decore[0] + decore[1];
		win->height += decore[2] + decore[3];
	}

	win->right = win->left + win->width;
	win->bottom = win->top + win->height;

	/* name */
	win->name[0] = 0;
	win_read_prop(xobj, win->id, "WM_NAME", sizeof(char), sizeof(win->name), &win->name);
	win->name[sizeof(win->name) - 1] = 0;

	/* monitor */
	xlib_win_match_monitor(xobj, win);
	(void)win_read_prop(xobj, id, "_NET_WM_DESKTOP", sizeof(int), 1, &win->desktop);

	/* visibility */
	win->visible = (attrs.map_state == IsViewable);

	return 0;
}

void xlib_win_destroy(xlib_win_t *win){
	for(size_t i=0; i<win->nchilds; i++)
		xlib_win_destroy(win->childs + i);

	free(win->childs);
}

int xlib_win_childs(xlib_t *xobj, xlib_win_t *win){
	int r = 0;
	Window dummy;
	Window *childs;


	XQueryTree(xobj->dpy, win->id, &dummy, &dummy, &childs, &win->nchilds);
	win->childs = calloc(win->nchilds, sizeof(xlib_win_t));

	for(size_t i=0; i<win->nchilds && r==0; i++)
		r |= xlib_win_init(xobj, childs[i], win->childs + i);

	if(childs != 0x0)
		XFree(childs);

	return r;
}

void xlib_win_match_monitor(xlib_t *xobj, xlib_win_t *win){
	xlib_monitor_t *mon;


	for(size_t i=0; i<xobj->nmonitors; i++){
		mon = xobj->monitors + i;

		if(win->left >= mon->left && win->left < mon->right && win->top >= mon->top && win->top < mon->bottom){
			win->monitor = i;
			return;
		}
	}
}

void xlib_win_info(xlib_win_t *win, log_lvl_t lvl, char const *indent){
	plog(lvl, __FILE__, __LINE__, "%swindow: id=%u, left=%d, top=%d, right=%d, bottom=%d\n"
		, indent
		, (int)win->id
		, win->left, win->top, win->right, win->bottom
	);
}

int xlib_win_focus(xlib_t *xobj, xlib_win_t *win){
	return -(XSetInputFocus(xobj->dpy, win->id, RevertToParent, CurrentTime) != 1);
}

int xlib_win_summon(xlib_t *xobj, xlib_win_t *win, int desktop, int x, int y){
	XEvent xev;
	XWindowAttributes attr;


	XGetWindowAttributes(xobj->dpy, win->id, &attr);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.display = xobj->dpy;
	xev.xclient.window = win->id;
	xev.xclient.message_type = XInternAtom(xobj->dpy, "_NET_WM_DESKTOP", False);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = desktop;
	xev.xclient.data.l[1] = 2;	// indicate message from a pager

	if(XSendEvent(xobj->dpy, attr.screen->root, False, SubstructureNotifyMask | SubstructureRedirectMask, &xev) == 0)
		return -1;

	win->desktop = desktop;

	return xlib_win_move(xobj, win, x, y, false);
}

int xlib_win_move(xlib_t *xobj, xlib_win_t *win, int x, int y, bool relative){
	if(relative){
		x += win->left;
		y += win->top;
	}

	if(XMoveWindow(xobj->dpy, win->id, x, y) == 0)
		return -1;

	win->left = x;
	win->top = y;
	win->right = win->left + win->width;
	win->bottom = win->top + win->height;

	return 0;
}

int xlib_win_map(xlib_t *xobj, xlib_win_t *win){
	return -(XMapWindow(xobj->dpy, win->id) == 0);
}

int xlib_win_unmap(xlib_t *xobj, xlib_win_t *win){
	return -(XUnmapWindow(xobj->dpy, win->id) == 0);
}

int xlib_mouse_moveto(xlib_t *xobj, int x, int y){
	return -(XWarpPointer(xobj->dpy, None, xobj->root, 0, 0, 0, 0, x, y) == 0);
}


/* local functions */
static int error_handler(Display *dsp, XErrorEvent *evt){
	return 0;
}

static int monitors_init(xlib_t *xobj){
	xlib_monitor_t *monitor;
	XRRScreenResources *res;
	XRROutputInfo *oinfo;
	XRRCrtcInfo *cinfo;


	xobj->monitors = 0x0;

	/* get xrand screen resource and allocate monitors */
	res = XRRGetScreenResources(xobj->dpy, xobj->root);

	xobj->nmonitors = res->ncrtc;
	xobj->monitors = malloc(sizeof(xlib_monitor_t) * xobj->nmonitors);

	if(xobj->monitors == 0x0)
		goto_err(end, "out of memory");

	/* init monitor with xrand crtc info */
	for(size_t i=0; i<xobj->nmonitors; i++){
		monitor = xobj->monitors + i;
		cinfo = XRRGetCrtcInfo(xobj->dpy, res, res->crtcs[i]);

		monitor->xid = res->crtcs[i];
		monitor->left = cinfo->x;
		monitor->top = cinfo->y;
		monitor->right = monitor->left + cinfo->width;
		monitor->bottom = monitor->top + cinfo->height;
		monitor->connected = false;

		XRRFreeCrtcInfo(cinfo);
	}

	/* match xrandr crtcs against outputs, query the connection state */
	for(int i=0; i<res->noutput; i++){
		oinfo = XRRGetOutputInfo(xobj->dpy, res, res->outputs[i]);

		for(size_t j=0; j<xobj->nmonitors; j++){
			if(xobj->monitors[j].xid == oinfo->crtc){
				xobj->monitors[j].connected = (oinfo->connection == 0);
				break;
			}
		}

		XRRFreeOutputInfo(oinfo);
	}

end:
	XRRFreeScreenResources(res);

	return -(xobj->monitors == 0x0);
}

static int win_read_prop(xlib_t *xobj, Window win, char const *name, int el_size, size_t max_el, void *prop){
	int format,
		expected_format;
	unsigned char *val;
	unsigned long nprops,
				  bytes_left;
	Atom atom;


	atom = XInternAtom(xobj->dpy, name, 1);

	if(atom == None)
		return -1;

	expected_format = (el_size > 4) ? 32 : el_size * 8;

	if(XGetWindowProperty(xobj->dpy, win, atom, 0, -1, false, AnyPropertyType, &atom, &format, &nprops, &bytes_left, &val) != Success)
		return -1;

	if(format != expected_format || bytes_left != 0 || nprops > max_el){
		XFree(val);
		return -1;
	}

	// for string properties, copy the null byte, which is always allocated by XGetWindowProperty
	if(format == 8 && nprops < max_el)
		nprops++;

	memcpy(prop, val, el_size * nprops);
	XFree(val);

	return 0;
}
