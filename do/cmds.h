#ifndef CMDS_H
#define CMDS_H


#include <xlib.h>
#include <do/opts.h>


/* types */
typedef struct cmd_t{
	char const *name;

	int (*hdlr)(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
	int required_args;
} cmd_t;


/* prototypes */
int cmd_screen_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_info(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_list(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_focus(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_map(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_unmap(xlib_t *xobj, xlib_win_t *win, opts_t *opts);
int cmd_win_move(xlib_t *xobj, xlib_win_t *win, opts_t *opts);


#endif // CMDS_H
