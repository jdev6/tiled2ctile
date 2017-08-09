CFLAGS = -Wall

OUT = tiled2ctile

all:
	$(CC) $(CFLAGS) *.c -o $(OUT)

install:
	install $(OUT) /usr/bin

debug: CFLAGS += -ggdb
debug: all
