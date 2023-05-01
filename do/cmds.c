#include <log.h>
#include <xlib.h>
#include <do/opts.h>


/* global functions */
int cmd_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	xlib_monitor_t *mon;


	for(size_t i=0; i<xobj->nmonitors; i++){
		mon = xobj->monitors + i;

		INFO("monitor%u: %dx%d %d,%d+%d,%d\n"
			, i
			, mon->right - mon->left
			, mon->bottom - mon->top
			, mon->left
			, mon->top
			, mon->right
			, mon->bottom
		);
	}

	INFO(
		"\n"
		"window id=%u\n"
		"    desktop: %d\n"
		"    monitor: %d\n"
		"    left,top: %d,%d\n"
		"    right,bottom: %d,%d\n"
		"    dimensions: %dx%d\n"
		, (int)win->id
		, win->desktop
		, win->monitor
		, win->left
		, win->top
		, win->right
		, win->bottom
		, win->width
		, win->height
	);

	return 0;
}

int cmd_focus(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_focus(xobj, win);
}

int cmd_map(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_map(xobj, win);
}

int cmd_unmap(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_unmap(xobj, win);
}

int cmd_move(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	if(opts->desktop == -1)
		return xlib_win_move(xobj, win, 5, 5);

	return xlib_win_summon(xobj, win, opts->desktop, win->left + 5, win->top + 5);
}
