#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//READS FROM tiled exported .csv and WRITEs ctile- compatiblle file

const char* HELP =
"tiled2ctile [map] [metadata] [tile page] [[out or stdout]]\n";

int count_chars(FILE*, char);

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
	if (argc == 5) {
		if (!(output = fopen(argv[4], "wb"))) {
			fprintf(stderr, "cant open %s", argv[4]);
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

	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	//NOW CHANGE TO BIG ENDIAN . format uses bigendian
	width = (width<<8) | (width>>8);
	height = (height<<8) | (height>>8);
	#endif

	//WRITE HEADER
	fwrite("ctile", 1, 5, output);
	fwrite(&width,  1, 2, output);
	fwrite(&height, 1, 2, output);
	uint8_t page8 = (uint8_t)page;
	fwrite(&page8,  1, 1, output);
	char metadata[16];
	memcpy(metadata, argv[2], 16);
	fwrite(metadata,1,16, output);

	//WRITE DATA
	fseek(input, 0, SEEK_SET);
	uint8_t id = 0;
	int c;
	int neg = 0;
	int total = 0;
	while ((c = fgetc(input)) != EOF) {
		fprintf(stderr, "[%c]", c);
		if (c == ',' || c == '\n') {
			//flush
			total++;
			if (neg) id = 0;
			fprintf(stderr, "final %X\n", id);
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
			fprintf(stderr, ".id before; %i, %c.", id, c);
			id = id * 10 + c-'0';
			fprintf(stderr, ".now %i.", id);
		} else if (c == '-') {
			neg = 1;
			fprintf(stderr, ".negate.");
		} else {
			fprintf(stderr, "uknown char %c\n", c);
		}
	}
	fprintf(stderr, "total elements: %i\n", total);
	fclose(output);
	fclose(input);
	
}


int count_chars(FILE* f, char match) {
	int count = 0, c;
	while ((c = fgetc(f)) != EOF) {
		if (c == match) count++;
	}
	return count;
}
