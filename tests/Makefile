TESTS = $(wildcard *.c)
CC = gccs
CFLAGS = -Wall -Wextra -I../
FRAMEWORKS = -framework IOKit -framework CoreFoundation

all:
	mkdir -p bin
	@for test in $(TESTS); do \
		$(CC) $(CFLAGS) $(FRAMEWORKS) $$test -o bin/$${test%.c}; \
		ldid -Sentitlements.plist bin/$${test%.c}; \
	done

clean:
	rm -rf bin