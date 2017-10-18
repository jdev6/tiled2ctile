#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "ctile/ctile.h"
#include "macros.h"

//READS FROM tiled exported .csv and WRITEs ctile- compatiblle file

const char* HELP =
"tiled2ctile [map] [metadata] [tile page] [[out or stdout]] [[extra (type+attributes)]]\n";

int count_chars(FILE*, char);

ctile_tile** tiles;

int main(int argc, char** argv) {
	if (argc < 4) {
		fprintf(stderr, HELP);
		return 1;
	}

	FILE* input;
	FILE* output = stdout;
	int page = atoi(argv[3]);
	if (page > 0xFF) {
		fprintf(stderr, "page from 0 to 0xFF ONLy\n");
		return 1;
	}

	input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "cant open %s\n", argv[1]);
		return 1;
	}
	if (argc >= 5) {
		if (!(output = fopen(argv[4], "wb"))) {
			fprintf(stderr, "cant open %s", argv[4]);
			return 1;
		}
	}

	FILE* extra = NULL;

	if (argc >= 6) {
		if (!(extra = fopen(argv[5], "r"))) {
			fprintf(stderr, "cant open %s", argv[5]);
			return 1;
		}
	}
	
	//HEIGHT: numberof newlines
	uint16_t height = (uint16_t)count_chars(input, '\n');
	fseek(input, 0, SEEK_SET);
	
	//WIdth: number of commas/height + 1
	int commas = count_chars(input, ',');
	uint16_t width = commas/height + 1;

	fprintf(stderr, "WIDTH: %i, HEIGHT: %i, COMMAS: %i\n", width, height ,commas);

	tiles = calloc(width*height, sizeof(ctile_tile));

	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	//NOW CHANGE TO BIG ENDIAN . format uses bigendian
	uint16_t width_be = (width<<8) | (width>>8);
	uint16_t height_be = (height<<8) | (height>>8);
	#endif

	//WRITE HEADER
	fwrite("ctile", 1, 5, output);
	fwrite(&width_be,  1, 2, output);
	fwrite(&height_be, 1, 2, output);
	uint8_t page8 = (uint8_t)page;
	fwrite(&page8,  1, 1, output);
	char metadata[16];
	memcpy(metadata, argv[2], 16);
	fwrite(metadata,1,16, output);

	int header_offset = ftell(output);

	//WRITE DATA
	fseek(input, 0, SEEK_SET);
	uint8_t id = 0;
	int c;
	int neg = 0;
	int total = 0;
	while ((c = fgetc(input)) != EOF) {
//		fprintf(stderr, "[%c]", c);
		if (c == ',' || c == '\n') {
			//flush
			total++;
			if (neg) id = 0;
//			fprintf(stderr, "final %X\n", id);
			fwrite(&id, 1, 1, output);
			//TYPE and ATTRIBUTE
			unsigned char one = 1;
			unsigned char nil = 0;
			fwrite(id == 0 ? &nil : &one, 1, 1, output);
			fwrite(&nil, 1, 1, output);

			id = 0;
			neg = 0;
			continue;
		}
		if (isdigit(c)) {
//			fprintf(stderr, ".id before; %i, %c.", id, c);
			id = id * 10 + c-'0';
//			fprintf(stderr, ".now %i.", id);
		} else if (c == '-') {
			neg = 1;
//			fprintf(stderr, ".negate.");
		} else {
			fprintf(stderr, "uknown char %c\n", c);
		}
	}
	fprintf(stderr, "total elements: %i\n", total);
	fclose(input);

	if (extra) {
		int x = 0, y = 0;

		char* line = NULL; size_t n = 0;
		while (!feof(extra) && !ferror(extra)) {
			getline(&line, &n, extra);
			#define errer() fprintf(stderr, "error at extras file.\n")

			int arg = 0;
			char arg_str[64] = "";
			
			if (*line == '#') continue; //comment
			
			if (sscanf(line, "tile %i %i", &x, &y) == 2);
			
			else if (sscanf(line, "include %s", arg_str)) {
				if (*arg_str) {
					char cmd[100];
					snprintf(cmd, sizeof(cmd), "gcc -E -DT2C %s", arg_str);
					FILE* f = popen(cmd, "r");
					if (!f) {
						fprintf(stderr, "CANT fopen macro file '%s': %s\n", arg_str, strerror(errno));
					} else {
						macro_read_file(f);
					}
				}

			} else if (sscanf(line, "type %i", &arg) || sscanf(line, "type $%s", arg_str)) {
				if (*arg_str) {
					macro_t* m = macro_search(arg_str);
					if (m) arg = m->val;
					else {
						fprintf(stderr, "unknow macro name %s\n", arg_str);
						arg = 0;
					}
				}
				fprintf(stderr, "TYPE %i of %i,%i\n", arg, x, y);
				fseek(output, header_offset + ((x+y*width)*3) + 1, SEEK_SET);
				fputc(arg, output);

			} else if (sscanf(line, "atts %i", &arg)) {
				fseek(output, header_offset + ((x+y*width)*3) + 2, SEEK_SET);
				fputc(arg, output);
			}
		}

		free(line);
	}

	fclose(output);
	if (extra) fclose(extra);	
}


int count_chars(FILE* f, char match) {
	int count = 0, c;
	while ((c = fgetc(f)) != EOF) {
		if (c == match) count++;
	}
	return count;
}
