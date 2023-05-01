#include <stdarg.h>
#include <stdio.h>
#include <log.h>


/* global variables */
log_lvl_t log_level = LOG_ERROR | LOG_INFO;


/* global functions */
void plog(log_lvl_t lvl, char const *file, size_t line, char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	vplog(lvl, file, line, fmt, lst);
	va_end(lst);
}

void vplog(log_lvl_t lvl, char const *file, size_t line, char const *fmt, va_list lst){
	FILE *fp;


	if((lvl & log_level) == 0)
		return;

	fp = (lvl == LOG_ERROR) ? stderr : stdout;

	if(lvl == LOG_ERROR)
		fprintf(fp, "%s:%zu:" COLOR("error", FG_RED) ": ", file, line);

	vfprintf(fp, fmt, lst);
}
