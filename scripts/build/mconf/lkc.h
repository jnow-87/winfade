/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 * Released under the terms of the GNU GPL v2.0.
 *
 * Note by Jan Nowotsch:
 * 	This code has been borrowed from the linux kernel build system.
 */

#ifndef LKC_H
#define LKC_H

#include "expr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define P(name,type,arg)	extern type name arg
#include "lkc_proto.h"
#undef P

#define SRCTREE "srctree"

#ifndef CONFIG_
# define CONFIG_ "CONFIG_"
#endif
static inline char const *CONFIG_prefix(void)
{
	return getenv( "CONFIG_" ) ?: CONFIG_;
}
#undef CONFIG_
#define CONFIG_ CONFIG_prefix()

#define TF_COMMAND	0x0001
#define TF_PARAM	0x0002
#define TF_OPTION	0x0004

enum conf_def_mode {
	def_default,
	def_yes,
	def_mod,
	def_no,
	def_random
};

#define T_OPT_MODULES		1
#define T_OPT_DEFCONFIG_LIST	2
#define T_OPT_ENV		3

struct kconf_id {
	int name;
	int token;
	unsigned int flags;
	enum symbol_type stype;
};

extern int zconfdebug;

int zconfparse(void);
void zconfdump(FILE *out);
void zconf_starthelp(void);
FILE *zconf_fopen(char const *name);
void zconf_initscan(char const *name);
void zconf_nextfile(char const *name);
int zconf_lineno(void);
char const *zconf_curname(void);

/* confdata.c */
char const *conf_get_configname(void);
char *conf_get_default_confname(void);
void sym_set_change_count(int count);
void sym_add_change_count(int count);
void conf_set_all_new_symbols(enum conf_def_mode mode);

struct conf_printer {
	void (*print_symbol)(FILE *, struct symbol *, char const *, void *);
	void (*print_comment)(FILE *, char const *, void *);
};

/* confdata.c and expr.c */
static inline void xfwrite(const void *str, size_t len, size_t count, FILE *out)
{
	assert(len != 0);

	if (fwrite(str, len, count, out) != count)
		fprintf(stderr, "Error in writing or end of file.\n");
}

/* menu.c */
void _menu_init(void);
void menu_warn(struct menu *menu, char const *fmt, ...);
struct menu *menu_add_menu(void);
void menu_end_menu(void);
void menu_add_entry(struct symbol *sym);
void menu_end_entry(void);
void menu_add_dep(struct expr *dep);
void menu_add_visibility(struct expr *dep);
struct property *menu_add_prop(enum prop_type type, char *prompt, struct expr *expr, struct expr *dep);
struct property *menu_add_prompt(enum prop_type type, char *prompt, struct expr *dep);
void menu_add_expr(enum prop_type type, struct expr *expr, struct expr *dep);
void menu_add_symbol(enum prop_type type, struct symbol *sym, struct expr *dep);
void menu_add_option(int token, char *arg);
void menu_finalize(struct menu *parent);
void menu_set_type(int type);

/* util.c */
struct file *file_lookup(char const *name);
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);

struct gstr {
	size_t len;
	char  *s;
	/*
	* when max_width is not zero long lines in string s (if any) get
	* wrapped not to exceed the max_width value
	*/
	int max_width;
};
struct gstr str_new(void);
struct gstr str_assign(char const *s);
void str_free(struct gstr *gs);
void str_append(struct gstr *gs, char const *s);
void str_printf(struct gstr *gs, char const *fmt, ...);
char const *str_get(struct gstr *gs);

/* symbol.c */
extern struct expr *sym_env_list;

void sym_init(void);
void sym_clear_all_valid(void);
void sym_set_all_changed(void);
void sym_set_changed(struct symbol *sym);
struct symbol *sym_choice_default(struct symbol *sym);
char const *sym_get_string_default(struct symbol *sym);
struct symbol *sym_check_deps(struct symbol *sym);
struct property *prop_alloc(enum prop_type type, struct symbol *sym);
struct symbol *prop_get_symbol(struct property *prop);
struct property *sym_get_env_prop(struct symbol *sym);

static inline tristate sym_get_tristate_value(struct symbol *sym)
{
	return sym->curr.tri;
}


static inline struct symbol *sym_get_choice_value(struct symbol *sym)
{
	return (struct symbol *)sym->curr.val;
}

static inline bool sym_set_choice_value(struct symbol *ch, struct symbol *chval)
{
	return sym_set_tristate_value(chval, yes);
}

static inline bool sym_is_choice(struct symbol *sym)
{
	return sym->flags & SYMBOL_CHOICE ? true : false;
}

static inline bool sym_is_choice_value(struct symbol *sym)
{
	return sym->flags & SYMBOL_CHOICEVAL ? true : false;
}

static inline bool sym_is_optional(struct symbol *sym)
{
	return sym->flags & SYMBOL_OPTIONAL ? true : false;
}

static inline bool sym_has_value(struct symbol *sym)
{
	return sym->flags & SYMBOL_DEF_USER ? true : false;
}

#ifdef __cplusplus
}
#endif

#endif /* LKC_H */
