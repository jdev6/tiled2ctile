#include <string.h>

#include "macros.h"

int macro_count = 0;

void macro_add(char* name, int val) {
	if (macro_count < MAX_MACROS) {
		macros[macro_count++] = {str, val};
	}
}

macro_t macro_search(char* s) {
	for (int i = 0; i < macro_count; i++) {
		if (strcmp(s, macros[i].name) == 0)
			return macros[i];
	}

	return NULL;
}
