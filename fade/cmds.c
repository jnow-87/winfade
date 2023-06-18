#include <unistd.h>
#include <log.h>
#include <xlib.h>
#include <fade/opts.h>
#include <fade/group.h>


/* types */
typedef enum{
	FADE_OUT = -1,
	FADE_IN = 1,
} fade_t;

typedef struct{
	int delta,
		min;
} delta_t;


/* local/static prototypes */
static delta_t delta(int win_low, int win_high, int mon_low, int mon_high);


/* global functions */
int cmd_select(xlib_t *xobj, group_t *group, opts_t *opts){
	xlib_win_t *win;


	for(size_t i=0; i<group->nwindows; i++){
		win = group->windows + i;

		if(win->id == xobj->focus){
			VERBOSE("remove window %u from group\n", (int)win->id);
			group->nwindows--;
			win->id = 0;

			goto end;
		}
	}

	VERBOSE("add window %u to group\n", (int)xobj->focus);

	// the memory for the group's window list is large enough to take one additional window
	if(xlib_win_init(xobj, xobj->focus, group->windows + group->nwindows) != 0)
		return ERROR("initialising window %d\n", xobj->focus);

	group->nwindows++;

end:
	return group_store(group);
}

int cmd_fade(xlib_t *xobj, group_t *group, opts_t *opts){
	delta_t dx[group->nwindows],
			dy[group->nwindows];
	fade_t dir;
	xlib_win_t *win;
	xlib_monitor_t *mon;


	if(group_store(group) != 0)
		return -1;

	dir = (group->windows[0].desktop != xobj->desktop) ? FADE_IN : FADE_OUT;
	VERBOSE("fade %s: steps=%zu, delay=%zu us\n", (dir == FADE_IN) ? "in" : "out", opts->steps, opts->delay_ms);

	/* calculate x, y movement per window */
	for(size_t i=0; i<group->nwindows; i++){
		win = group->windows + i;
		mon = xobj->monitors + win->monitor;

		dx[i] = delta(win->left, win->right, mon->left, mon->right);
		dy[i] = delta(win->top, win->bottom, mon->top, mon->bottom);

		xlib_win_info(win, LOG_VERBOSE, "  ");
		VERBOSE(
			"  monitor: left=%d, top=%d, right=%d, bottom=%d\n"
			"    dx: delta=%d, min=%d\n"
			"    dy: delta=%d, min=%d\n"
			, mon->left, mon->top, mon->right, mon->bottom
			, dx[i].delta, dx[i].min
			, dy[i].delta, dy[i].min
		);

		// only move to the border with the min distance
		if(dx[i].min < dy[i].min && dy[i].min > 0)		dy[i].delta = 0;
		else if(dy[i].min < dx[i].min && dx[i].min > 0)	dx[i].delta = 0;

		dx[i].delta /= (ssize_t)opts->steps * dir;
		dy[i].delta /= (ssize_t)opts->steps * dir;

		VERBOSE("    inc: x=%d, y=%d\n", dx[i].delta , dy[i].delta);
	}

	/* update windows */
	// fade-in prologue
	for(size_t i=0; i<group->nwindows && dir==FADE_IN; i++){
		win = group->windows + i;

		// move the window onto the current desktop
		// it also needs to be moved to its faded-out position, since unmap
		// during fade-out changes its position
		xlib_win_summon(xobj, win, xobj->desktop, win->left - dx[i].delta * opts->steps, win->top - dy[i].delta * opts->steps);
		xlib_win_map(xobj, win);
	}

	// fade
	for(size_t i=1; i<=opts->steps; i++){
		for(size_t j=0; j<group->nwindows; j++)
			xlib_win_move(xobj, group->windows + j, dx[j].delta, dy[j].delta, true);

		xlib_sync(xobj);
		usleep(opts->delay_ms * 1000);
	}

	// fade-out epilogue
	for(size_t i=0; i<group->nwindows && dir==FADE_OUT; i++)
		xlib_win_unmap(xobj, group->windows + i);

	xlib_sync(xobj);

	return 0;
}

int cmd_dump(xlib_t *xobj, group_t *group, opts_t *opts){
	for(size_t i=0; i<group->nwindows; i++)
		xlib_win_info(group->windows + i, LOG_INFO, "");

	return 0;
}


/* local functions */
static delta_t delta(int win_low, int win_high, int mon_low, int mon_high){
	if(win_low - mon_low < mon_high - win_high)
		return (delta_t){ .delta = win_high - mon_low, .min = win_low - mon_low };

	return (delta_t){ .delta = win_low - mon_high, .min = mon_high - win_high };
}
