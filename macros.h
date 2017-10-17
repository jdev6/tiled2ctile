#ifndef MACROS_H
#define MACROS_H

typedef struct {
	char* name;
	int val;
} macro_t;

#define MAX_MACROS 256
macro_t macros[MAX_MACROS];
int macro_count;

void macro_add(char*, int);
macro_t macro_search(char*);

#endif
