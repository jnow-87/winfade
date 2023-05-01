#include <xlib.h>


/* global functions */
int main(void){
	int r = 0;
	xlib_t xobj;
	xlib_win_t focus;


	if(xlib_init(&xobj, 0x0) != 0)
		return 1;

	r |= xlib_win_init(&xobj, xobj.focus, &focus);
	r |= xlib_mouse_moveto(&xobj, focus.right, focus.bottom);

	xlib_sync(&xobj);
	xlib_destroy(&xobj);

	return -r;
}
