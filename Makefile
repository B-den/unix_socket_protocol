
CC="gcc"
IGNORE="./tools"
FSANITIZER="-fsanitize=address"
CFLAGS="-O2"


format: .clang-format
	find . -path $(IGNORE) -prune -o -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format -style=file -i {} \;

compile:
	$(CC) -o client client.c send.c receive.c tools/crcspeed/crc64speed.c tools/crcspeed/crcspeed.c $(CFLAGS) $(FSANITIER); \
	$(CC) -o server server.c send.c receive.c tools/crcspeed/crc64speed.c tools/crcspeed/crcspeed.c $(CFLAGS) $(FSANITIER)

client: compile
	./client

server: compile
	./server &

all: server client

example:
	$(CC) -o r examples/receiver.c $(CFLAGS) $(FSANITIZER); $(CC) -o s examples/sender.c $(CFLAGS) $(FSANITIZER)
