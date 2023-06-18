#include <stdlib.h>
#include <log.h>
#include <xlib.h>
#include <do/opts.h>


/* global functions */
int cmd_win_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
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

int cmd_win_focus(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_focus(xobj, win);
}

int cmd_win_map(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_map(xobj, win);
}

int cmd_win_unmap(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	return xlib_win_unmap(xobj, win);
}

int cmd_win_move(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	int x,
		y;


	if(opts->cmd_argc != 2)
		return ERROR("missing arguments: <x> <y>\n");

	x = atoi(opts->cmd_argv[0]);
	y = atoi(opts->cmd_argv[1]);

	if(opts->desktop == -1)
		return xlib_win_move(xobj, win, x, y, opts->relative);

	return xlib_win_summon(xobj, win, opts->desktop, win->left + x, win->top + y);
}
