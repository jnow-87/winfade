/**
 * Copyright (C) 2015 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Most of source code is taken from the linux kernel's build helper fixdep.c
 * but has been restructured for the purpose of this project.
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef HASHTBL_H
#define HASHTBL_H


/* types */
struct item{
	struct item *next;
	unsigned int len,
				 hash;
	char name[0];
};


/* prototypes */
int hashtbl_lookup(char const *name, int n, unsigned int hash);
int hashtbl_add(char const *name, int n);
void hashtbl_clear(void);


#endif // HASHTBL_H
