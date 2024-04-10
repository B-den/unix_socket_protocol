
CC="gcc"
IGNORE="./tools"
FSANITIZER="-fsanitize=address"
CFLAGS="-lpthread"

format: .clang-format
	find . -path $(IGNORE) -prune -o -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format -style=file -i {} \;

test: test.c


all: format test

example:
	$(CC) -o r examples/receiver.c $(CFLAGS) $(FSANITIZER); $(CC) -o s examples/sender.c $(CFLAGS) $(FSANITIZER)
