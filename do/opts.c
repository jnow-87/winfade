#include <version.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include <log.h>
#include <do/opts.h>
#include <do/cmds.hash.h>


/* macros */
#define PROGNAME			"windo"

#define DEFAULT_WIN			0
#define DEFAULT_DESKTOP		-1

#define DEFAULT_OPTS (opts_t){ \
	.win = DEFAULT_WIN, \
	.desktop = DEFAULT_DESKTOP, \
	.cmd = 0x0, \
}


/* local/static prototypes */
static int help(char const *err, ...);
static int version(void);


/* global functions */
int opts_parse(int argc, char **argv, opts_t *opts){
	int opt;
	struct option const long_opt[] = {
		{ .name = "id",			.has_arg = required_argument,	.flag = 0,	.val = 'i' },
		{ .name = "desktop",	.has_arg = required_argument,	.flag = 0,	.val = 'd' },
		{ .name = "version",	.has_arg = no_argument,			.flag = 0,	.val = 'V' },
		{ .name = "help",		.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0 }
	};


	*opts = DEFAULT_OPTS;

	while((opt = getopt_long(argc, argv, ":i:d:Vh", long_opt, 0)) != -1){
		switch(opt){
		case 'i':	opts->win = atoi(optarg); break;
		case 'd':	opts->desktop = atoi(optarg); break;
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
		"    %-10.10s    %s\n"
		"    %-10.10s    %s\n"
		"\n"
		"Options:\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		, "info", "print info on xlib and the target window"
		, "focus", "move focus to the target window"
		, "map", "map the target window"
		, "unmap", "unmap the target window"
		, "move", "move the target window: x+=5, y+=5, desktop according to --desktop"
		, "-i, --id=<win>", "id of target window, default is the window in focus" DEFAULT(DEFAULT_WIN)
		, "-d, --desktop=<num>", "target desktop number, default is the current one" DEFAULT(DEFAULT_DESKTOP)
		, "-V, --version", "print version"
		, "-h, --help", "print this message"
	);

	return (err == 0x0) ? 1 : -1;
}

static int version(void){
	INFO(PROGNAME" version:\n%s\n", VERSION);

	return 1;
}
