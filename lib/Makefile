CC = clang
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC

SRC = libkextrw.c
OBJ = $(SRC:.c=.o)
LIB = libkextrw.a

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	rm -f $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)