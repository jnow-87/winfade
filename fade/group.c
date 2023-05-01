#include <config/config.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <log.h>
#include <xlib.h>
#include <fade/group.h>


/* macros */
#define sizeof_mem(type, member)	sizeof(((type*)(0))->member)


/* types */
typedef struct{
	size_t nwindows;
} header_t;

typedef struct{
	unsigned long int id;
	int left,
		top,
		right,
		bottom;
} win_t;


/* local/static prototypes */
static group_t *alloc_group(char const *file, size_t n);


/* global functions */
// NOTE	the allocated group has memory for one further window, cf. alloc_group()
group_t *group_load(size_t num, xlib_t *xobj){
	group_t *group = 0x0;
	char file[sizeof_mem(group_t, file)];
	int fd;
	void *data;
	struct stat fstat;
	header_t *hdr;
	win_t *wdata;
	xlib_win_t *win;


	snprintf(file, sizeof(file), CONFIG_GROUP_FILE_PATTERN, num);
	file[sizeof(file) - 1] = 0;

	VERBOSE("loading group %zu from %s\n", num, file);
	fd = open(file, O_RDONLY);

	if(fd == -1){
		if(errno == ENOENT)
			return alloc_group(file, 0);

		goto end_0;
	}

	if(stat(file, &fstat))
		goto end_0;

	data = mmap(0x0, fstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(data == MAP_FAILED)
		goto end_1;

	hdr = (header_t*)data;
	group = alloc_group(file, hdr->nwindows);

	if(group == 0x0)
		goto end_2;

	VERBOSE("loading %zu window(s)\n", hdr->nwindows);
	group->nwindows = 0;

	for(size_t i=0; i<hdr->nwindows; i++){
		wdata = (win_t*)(data + sizeof(header_t) + i * sizeof(win_t));
		win = group->windows + group->nwindows;

		if(xlib_win_init(xobj, wdata->id, win) == 0){
			// reset window position to original position, when it was last
			// mapped, i.e. visible on a desktop
			if(win->desktop == -1){
				win->left = wdata->left;
				win->top = wdata->top;
				win->right = wdata->right;
				win->bottom = wdata->bottom;

				xlib_win_match_monitor(xobj, win);
			}

			xlib_win_info(win, LOG_VERBOSE, "  ");
			group->nwindows++;
		}
		else
			VERBOSE("  window doesn't exist anymore: id=%u\n", (int)win->id);
	}


end_2:
	munmap(data, fstat.st_size);

end_1:
	close(fd);

end_0:
	if(group == 0x0)
		STRERROR("loading group file %s", file);

	return group;
}

int group_store(group_t *group){
	int r = 0;
	int fd;
	win_t wdata;
	xlib_win_t *win;


	fd = open(group->file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if(fd == -1)
		return STRERROR("opening group file %s", group->file);

	VERBOSE("writing %zu window(s)\n", group->nwindows);
	r |= (write(fd, &group->nwindows, sizeof(size_t)) != sizeof(size_t));

	for(size_t i=0; i<group->nwindows; i++){
		win = group->windows + i;

		if(win->id == 0){
			VERBOSE("  ignore zero-id window %zu\n", i);
			continue;
		}

		wdata.id = win->id;
		wdata.left = win->left;
		wdata.top = win->top;
		wdata.right = win->right;
		wdata.bottom = win->bottom;

		xlib_win_info(win, LOG_VERBOSE, "  ");
		r |= (write(fd, &wdata, sizeof(win_t)) != sizeof(win_t));
	}

	close(fd);

	if(r != 0)
		STRERROR("writing group file %s", group->file);

	return -r;
}


/* local functions */
static group_t *alloc_group(char const *file, size_t n){
	group_t *group;


	// NOTE allocate group with space for one additional window in case a
	// 		command wants to extend the list
	group = malloc(sizeof(group_t) + sizeof(xlib_win_t) * (n + 1));

	if(group == 0x0)
		return 0x0;

	group->nwindows = n;
	strcpy(group->file, file);

	return group;
}
