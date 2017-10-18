#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>

typedef struct {
	char* name;
	int val;
} macro_t;

#define MAX_MACROS 256
macro_t macros[MAX_MACROS];
int macro_count;

void macro_add(char*, int);
macro_t* macro_search(char*);
void macro_read_file(FILE* f);

#endif
