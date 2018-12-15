all:main

SRC = $(wildcard *.c)
OBJ = $(patsubst %c, %o, $(SRC))

LIBS = -lmainloop -L.

main:$(OBJ)
	gcc -o $@ $^ $(LIBS)

.PHONY:all clean

clean:
	-rm -f main $(OBJ)


