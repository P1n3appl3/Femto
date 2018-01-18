TARGET = femto
CC = clang
CFLAGS=-Wall -Wextra -pedantic -std=c99 -g

.PHONY: default all clean

default: $(TARGET)
all: default

SRC = $(wildcard src/*.c)
INC = $(wildcard inc/*.h)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

.PRECIOUS: obj/%.o
obj/%.o: src/%.c $(INC)
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -Wall -g -o $@

install:
	$(eval PREFIX ?= /usr)
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin

uninstall:
	$(eval PREFIX ?= /usr)
	rm $(PREFIX)/bin/$(TARGET)

clean:
	-rm -f obj/*.o
	-rm -f $(TARGET)
