#ifndef OPTS_H
#define OPTS_H


#include <stdbool.h>
#include <X11/Xlib.h>


/* incomplete types */
struct cmd_t;


/* types */
typedef struct{
	struct cmd_t const *cmd;

	size_t group,
		   steps,
		   delay_ms;
	bool verbose;
} opts_t;


/* prototypes */
int opts_parse(int argc, char **argv, opts_t *opts);


#endif // OPTS_H
