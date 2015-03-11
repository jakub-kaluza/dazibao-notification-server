#Nicolas CAILLOUX, Jakub KALUZA, Pawel MLYNARSKI
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
READER=reader
WRITER=writer
DELETER=deleter
COMPACTER=compacter
SERVER=server
CLIENT=client
CC=gcc
CFLAGS := -Wall -Wextra -pedantic-errors -g

all: $(READER) $(WRITER) $(DELETER) $(SERVER) $(CLIENT) $(COMPACTER)

show:
	@echo "sources : "$(SOURCES);
	@echo "objects : "$(OBJECTS);

$(READER) $(WRITER) $(DELETER) $(COMPACTER): helpers.o dazibao.o

$(READER): $(READER).o
	$(CC) $(CFLAGS) $^ -o $@

$(WRITER): $(WRITER).o
	$(CC) $(CFLAGS) $^ -o $@

$(DELETER): $(DELETER).o
	$(CC) $(CFLAGS) $^ -o $@

$(COMPACTER): $(COMPACTER).o
	$(CC) $(CFLAGS) $^ -o $@

$(NOTIFIER): $(SERVER) $(CLIENT)

$(SERVER):
	cd notifier; make server;
	cp notifier/$(SERVER) $(SERVER)

$(CLIENT):
	cd notifier; make client;
	cp notifier/$(CLIENT) $(CLIENT)

%.o:%.d
	$(CC) -c $(CFLAGS) $^

%.d:%.c %.h
	$(CC) -MM $(CFLAGS) $^

clean:
	rm -f $(OBJECTS) *.d *~ *#
	cd notifier; make clean

proper: clean
	rm -f $(READER) $(WRITER) $(DELETER) $(COMPACTER) $(SERVER) $(CLIENT)
	cd notifier; make proper

