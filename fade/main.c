#include <stdlib.h>
#include <stdio.h>
#include <log.h>
#include <xlib.h>
#include <fade/opts.h>
#include <fade/group.h>
#include <fade/cmds.h>


/* global functions */
int main(int argc, char **argv){
	int r = -1;
	opts_t opts;
	xlib_t xobj;
	group_t *group;


	/* init */
	if(opts_parse(argc, argv, &opts) != 0)
		return 1;

	if(opts.verbose)
		log_level |= LOG_VERBOSE;

	if(xlib_init(&xobj, 0x0) != 0)
		goto end_0;

	/* load group file */
	group = group_load(opts.group, &xobj);

	if(group == 0x0)
		goto end_1;

	/* exec command */
	r = opts.cmd->hdlr(&xobj, group, &opts);

	free(group);

end_1:
	xlib_destroy(&xobj);

end_0:
	return -r;
}
