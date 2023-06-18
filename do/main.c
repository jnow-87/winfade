#include <log.h>
#include <xlib.h>
#include <do/opts.h>
#include <do/cmds.h>


/* global functions */
int main(int argc, char **argv){
	int r;
	opts_t opts;
	xlib_t xobj;
	xlib_win_t win;


	if(opts_parse(argc, argv, &opts) != 0)
		return 1;

	if(xlib_init(&xobj, 0x0) != 0)
		return 1;

	win.id = (opts.win == -1) ? xobj.root : ((opts.win != 0) ? (Window)opts.win : xobj.focus);

	if(xlib_win_init(&xobj, win.id, &win) == 0){
		r = opts.cmd->hdlr(&xobj, &win, &opts);
		xlib_sync(&xobj);
	}
	else
		r = ERROR("initialising window %d\n", win.id);

	xlib_win_destroy(&win);
	xlib_destroy(&xobj);

	return -r;
}
