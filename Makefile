READER=reader
WRITER=writer
DELETER=deleter
COMPACTER=compacter
SERVER=server
CLIENT=client
EXEC=$(READER) $(WRITER) $(DELETER) $(SERVER) $(CLIENT) $(COMPACTER)

all: sub

sub:
	cd src;	make
	cp src/$(READER) ./
	cp src/$(WRITER) ./
	cp src/$(DELETER) ./
	cp src/$(SERVER) ./
	cp src/$(CLIENT) ./
	cp src/$(COMPACTER) ./

clean:
	cd src;	make clean;

proper: clean
	rm $(EXEC)
	cd src; make proper;
