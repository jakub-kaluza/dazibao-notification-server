CC=gcc
CFLAGS=-Wall -Wextra -g
DEPS = dzbNotify.h
SERVER=server
CLIENT=client

all: $(SERVER) $(CLIENT)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SERVER): dzbNotify.o list.o utilities.o monitorDazibao.o addReader.o
	$(CC) $(CFLAGS) $^ -o $(SERVER) -I. -lpthread

$(CLIENT):client.c
	$(CC) $(CFLAGS) $^ -o $@
clean:
	rm -f *.o *.d *~ *#

proper:clean
	rm $(SERVER) $(CLIENT)
