
CC="gcc"
IGNORE="./tools"

format: .clang-format
	find . -path $(IGNORE) -prune -o -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format -style=file -i {} \;

test: test.c


all: format test


