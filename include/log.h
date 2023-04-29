#ifndef LOG_H
#define LOG_H


#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>


/* macros */
// foreground colors
#define FG_BLACK	"\033[30m"
#define FG_RED		"\033[31m"
#define FG_GREEN	"\033[32m"
#define FG_YELLOW	"\033[33m"
#define FG_BLUE		"\033[34m"
#define FG_VIOLETT	"\033[35m"
#define FG_KOBALT	"\033[36m"
#define FG_WHITE	"\033[37m"

// background colors
#define BG_BLACK	"\033[40m"
#define BG_RED		"\033[41m"
#define BG_GREEN	"\033[42m"
#define BG_YELLOW	"\033[43m"
#define BG_BLUE		"\033[44m"
#define BG_VIOLETT	"\033[45m"
#define BG_KOBALT	"\033[46m"
#define BG_WHITE	"\033[47m"

// controls
#define RESET_ATTR	"\033[0m"

// color control
#define ATTR(s, attr)	attr s RESET_ATTR
#define COLOR			ATTR

// stringification
#define STRINGIFY(s)	#s
#define DEFAULT(s)		" (default: " STRINGIFY(s) ")"

// log macros
#define goto_err(label, fmt, ...)	{ plog(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); goto label; }
#define ERROR(fmt, ...)				({ plog(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); -1; })
#define STRERROR(fmt, ...)			ERROR(fmt ": %s\n", ##__VA_ARGS__, strerror(errno))
#define INFO(fmt, ...)				plog(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define VERBOSE(fmt, ...)			plog(LOG_VERBOSE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


/* types */
typedef enum{
	LOG_ERROR = 0x1,
	LOG_INFO = 0x2,
	LOG_VERBOSE = 0x4,
} log_lvl_t;


/* prototypes */
void plog(log_lvl_t lvl, char const *file, size_t line, char const *fmt, ...);
void vplog(log_lvl_t lvl, char const *file, size_t line, char const *fmt, va_list lst);


/* external variables */
extern log_lvl_t log_level;


#endif // LOG_H
