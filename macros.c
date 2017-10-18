#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "macros.h"

int macro_count = 0;

void macro_add(char* name, int val) {
	if (macro_count < MAX_MACROS) {
		macros[macro_count].val = val;
		macros[macro_count].name = malloc(sizeof(name)+1);
		strcpy(macros[macro_count++].name, name);
	}
}

macro_t* macro_search(char* s) {
	for (int i = 0; i < macro_count; i++) {
		if (strcmp(s, macros[i].name) == 0)
			return &macros[i];
	}

	return NULL;
}

void macro_read_file(FILE* f) {
/*	{int c=0; while((c=getc(f))!=EOF) putchar(c);}
	fseek(f,0,SEEK_SET);*/

	char* line = NULL; size_t n = 0;
	int def_offset = 0;

	while (!feof(f) && !ferror(f)) {
		getline(&line, &n, f);
		char line2[strlen(line)+1];
		strcpy(line2, line);
		
		int arg = 0;
		char arg_str[64] = "";
		
		if (*line == '#' || *line == '\0' || *line == ' ') continue; //comment
		
		char* s = strtok(line2, ";");
		while (s) {
			if (sscanf(s, "T2C_DEF_OFFSET %i;", &def_offset) == 1);
			else if (sscanf(s, " T2C_DEF %s %i;", arg_str, &arg) == 2) {
				fprintf(stderr, "ADD macro <%s = %i>\n", arg_str, arg+def_offset);
				macro_add(arg_str, arg+def_offset);
			}

			s = strtok(NULL, ";");
//			if (s) fprintf(stderr, "%s;\n", s);
		}
		free(line);
	}
}
