#ifndef CMDS_H
#define CMDS_H


#include <xlib.h>
#include <fade/opts.h>
#include <fade/group.h>


/* types */
typedef struct cmd_t{
	char const *name;

	int (*hdlr)(xlib_t *xobj, group_t *group, opts_t *opts);
} cmd_t;


/* prototypes */
int cmd_select(xlib_t *xobj, group_t *group, opts_t *opts);
int cmd_fade(xlib_t *xobj, group_t *group, opts_t *opts);
int cmd_dump(xlib_t *xobj, group_t *group, opts_t *opts);


#endif // CMDS_H
