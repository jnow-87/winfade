#include <config/config.h>
#include <version.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include <log.h>
#include <fade/opts.h>
#include <fade/cmds.hash.h>


/* macros */
#define PROGNAME			"winfade"

#define DEFAULT_GROUP		0
#define DEFAULT_STEPS		CONFIG_ANIM_STEPS
#define DEFAULT_DELAY_MS	CONFIG_ANIM_DELAY_MS
#define DEFAULT_VERBOSE		false

#define DEFAULT_OPTS (opts_t){ \
	.cmd = 0x0, \
	.group = DEFAULT_GROUP, \
	.steps = DEFAULT_STEPS, \
	.delay_ms = DEFAULT_DELAY_MS, \
	.verbose = DEFAULT_VERBOSE, \
}


/* local/static prototypes */
static int help(char const *err, ...);
static int version(void);


/* global functions */
int opts_parse(int argc, char **argv, opts_t *opts){
	int opt;
	struct option const long_opt[] = {
		{ .name = "group",		.has_arg = required_argument,	.flag = 0,	.val = 'g' },
		{ .name = "steps",		.has_arg = required_argument,	.flag = 0,	.val = 's' },
		{ .name = "delay",		.has_arg = required_argument,	.flag = 0,	.val = 'd' },
		{ .name = "verbose",	.has_arg = no_argument,			.flag = 0,	.val = 'v' },
		{ .name = "version",	.has_arg = no_argument,			.flag = 0,	.val = 'V' },
		{ .name = "help",		.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0 }
	};


	*opts = DEFAULT_OPTS;

	while((opt = getopt_long(argc, argv, ":g:s:d:vVh", long_opt, 0)) != -1){
		switch(opt){
		case 'g':	opts->group = atol(optarg); break;
		case 's':	opts->steps = atol(optarg); break;
		case 'd':	opts->delay_ms = atol(optarg); break;
		case 'v':	opts->verbose = true; break;
		case 'V':	return version();
		case 'h':	return help(0x0);
		case ':':	return help("missing argument to %s\n", argv[optind - 1]);
		case '?':	return help("invalid option %s\n", argv[optind - 1]);
		default:	return help("unknown error\n");
		}
	}

	if(argc - optind != 1)
		return help("invalid number of arguments\n");

	opts->cmd = cmds_lookup(argv[optind], strlen(argv[optind]));

	if(opts->cmd == 0x0)
		return help("unknown command %s\n", argv[optind]);

	return 0;
}


/* local functions */
static int help(char const *err, ...){
	va_list lst;


	if(err != 0x0 && *err != 0){
		va_start(lst, err);
		vplog(LOG_ERROR, "", 0, err, lst);
		va_end(lst);
	}

	INFO(
		"usage: " PROGNAME " [options] <command>\n"
		"\n"
		"Commands:\n"
		"    %-10.10s    %s\n"
		"    %-10.10s    %s\n"
		"    %-10.10s    %s\n"
		"\n"
		"Options:\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		, "select", "add/remove the focused window to group"
		, "fade", "fade group in/out"
		, "dump", "print group information"
		, "-g, --group=<group>", "set target group number" DEFAULT(DEFAULT_GROUP)
		, "-s, --steps=<num>", "number of animation steps" DEFAULT(DEFAULT_STEPS)
		, "-d, --delay=<ms>", "delay between each animation step [ms]" DEFAULT(DEFAULT_DELAY_MS)
		, "-v, --verbose", "enable verbose output" DEFAULT(DEFAULT_VERBOSE)
		, "-V, --version", "print version"
		, "-h, --help", "print this message"
	);

	return (err == 0x0) ? 1 : -1;
}

static int version(void){
	INFO(PROGNAME" version:\n%s\n", VERSION);

	return 1;
}
