CC=gcc
CFLAGS=-Wall -Werror -Wextra
SOURCES=calculator.c
LIBS=-lm

all: calculator

calculator:
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LIBS)

clean:
	rm calculator

rebuild: clean all

.PHONY = all clean rebuild
