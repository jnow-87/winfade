#ifndef GROUP_H
#define GROUP_H


#include <xlib.h>


/* types */
typedef struct{
	char file[64];
	size_t nwindows;
	xlib_win_t windows[];
} group_t;


/* prototypes */
group_t *group_load(size_t num, xlib_t *xobj);
int group_store(group_t *group);


#endif // GROUP_H
