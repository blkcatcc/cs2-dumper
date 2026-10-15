CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -D_GNU_SOURCE
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
BIN := linux-dumper

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c src/dumper.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
