#include <stdlib.h>
#include <log.h>
#include <xlib.h>
#include <do/opts.h>


/* global functions */
int cmd_screen_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
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

	return 0;
}

int cmd_win_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	for(int i=0; i<opts->cmd_argc; i++){
		if(strcmp(opts->cmd_argv[i], "id") == 0)			INFO("%u ", (unsigned int)win->id);
		else if(strcmp(opts->cmd_argv[i], "name") == 0)		INFO("%s ", win->name);
		else if(strcmp(opts->cmd_argv[i], "desktop") == 0)	INFO("%d ", win->desktop);
		else if(strcmp(opts->cmd_argv[i], "monitor") == 0)	INFO("%d ", win->monitor);
		else if(strcmp(opts->cmd_argv[i], "position") == 0)	INFO("%d %d ", win->left, win->top);
		else if(strcmp(opts->cmd_argv[i], "geometry") == 0)	INFO("%u %u ", win->width, win->height);
		else												return ERROR("unkown property: %s", opts->cmd_argv[i]);
	}

	if(opts->cmd_argc == 0){
		INFO("id: %u\n"
			 "name: %d\n"
			 "desktop: %d\n"
			 "monitor: %d\n"
			 "position: %d %d\n"
			 "geometry: %dx%d\n"
			 , (unsigned int)win->id
			 , win->name
			 , win->desktop
			 , win->monitor
			 , win->left, win->top
			 , win->width, win->height
		);
	}
	else
		INFO("\n");

	return 0;
}

int cmd_win_list(xlib_t *xobj, xlib_win_t *win, opts_t *opts){
	// NOTE checking for a valid desktop mirrors the behaviour of xdotool
	if(win->name[0] != 0 && win->desktop != -1 && (!opts->only_visible || win->visible))
		INFO("%u\n", (unsigned int)win->id);

	if(xlib_win_childs(xobj, win) != 0)
		return -1;

	for(size_t i=0; i<win->nchilds; i++)
		cmd_win_list(xobj, win->childs + i, opts);

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
