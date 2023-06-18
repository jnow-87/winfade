#include <version.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include <log.h>
#include <do/opts.h>
#include <do/cmds.hash.h>


/* macros */
#define PROGNAME				"xdo"

#define DEFAULT_WIN				0
#define DEFAULT_DESKTOP			-1
#define DEFAULT_RELATIVE		false

#define DEFAULT_OPTS (opts_t){ \
	.win = DEFAULT_WIN, \
	.desktop = DEFAULT_DESKTOP, \
	.cmd = 0x0, \
	.cmd_argc = 0, \
	.cmd_argv = 0x0, \
	.relative = DEFAULT_RELATIVE, \
}


/* local/static prototypes */
static int help(char const *err, ...);
static int version(void);


/* global functions */
int opts_parse(int argc, char **argv, opts_t *opts){
	int opt;
	cmd_t const *cmd;
	struct option const long_opt[] = {
		{ .name = "id",				.has_arg = required_argument,	.flag = 0,	.val = 'i' },
		{ .name = "desktop",		.has_arg = required_argument,	.flag = 0,	.val = 'd' },
		{ .name = "relative",		.has_arg = no_argument,			.flag = 0,	.val = 'r' },
		{ .name = "version",		.has_arg = no_argument,			.flag = 0,	.val = 'V' },
		{ .name = "help",			.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0 }
	};


	*opts = DEFAULT_OPTS;

	while((opt = getopt_long(argc, argv, "+:i:d:rVh", long_opt, 0)) != -1){
		switch(opt){
		case 'i':	opts->win = atoll(optarg); break;
		case 'd':	opts->desktop = atoi(optarg); break;
		case 'r':	opts->relative = true; break;
		case 'V':	return version();
		case 'h':	return help(0x0);
		case ':':	return help("missing argument to %s\n", argv[optind - 1]);
		case '?':	return help("invalid option %s\n", argv[optind - 1]);
		default:	return help("unknown error\n");
		}
	}

	cmd = cmds_lookup(argv[optind], strlen(argv[optind]));

	if(cmd == 0x0)
		return help("unknown command %s\n", argv[optind]);

	opts->cmd = cmd;
	opts->cmd_argc = argc -optind - 1;
	opts->cmd_argv = argv + optind + 1;

	if(cmd->required_args >= 0 && opts->cmd_argc != cmd->required_args)
		return help("invalid number of <command> arguments, expected %d\n", cmd->required_args);

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
		"usage: " PROGNAME " [options] <command> <args>\n"
		"\n"
		"Commands:\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"    %-25.25s    %s\n"
		"\n"
		"Options:\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		"\n"
		"    %-20.20s    %s\n"
		"    %-20.20s    %s\n"
		, "screen-info", "print information on connected screens"
		, "win-info [{<property>}]", "print target window information"
		, "win-focus", "move focus to the target window"
		, "win-map", "map the target window"
		, "win-unmap", "unmap the target window"
		, "win-move <x> <y>", "move the target window to <x>, <y> and the given desktop, cf. --desktop"
		, "-i, --id=<win>", "id of target window, default is the window in focus" DEFAULT(DEFAULT_WIN)
		, "-d, --desktop=<num>", "target desktop number, default is the current one" DEFAULT(DEFAULT_DESKTOP)
		, "-r, --relative", "make operation relative to the window's current properties" DEFAULT(DEFAULT_RELATIVE)
		, "-V, --version", "print version"
		, "-h, --help", "print this message"
	);

	return (err == 0x0) ? 1 : -1;
}

static int version(void){
	INFO(PROGNAME" version:\n%s\n", VERSION);

	return 1;
}
