ROOT = .
TARGET  = PrimaryServer
BUILDIR = $(ROOT)/build
SRC = src
BINS= PrimaryServer BackupServer Client
INCLUDE = include
DEBUG = Y

CFLAGS += -std=gnu89 -Wall
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY:	clean all

all: $(BINS)

PrimaryServer: $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/coupling.o $(BUILDIR)/lazy.o $(BUILDIR)/intset.o $(BUILDIR)/PrimaryServer.o
	$(CC) $(CFLAGS) $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/lazy.o $(BUILDIR)/coupling.o $(BUILDIR)/intset.o $(BUILDIR)/PrimaryServer.o -lpthread -o $(ROOT)/PrimaryServer $(LDFLAGS)

BackupServer: $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/coupling.o $(BUILDIR)/lazy.o $(BUILDIR)/intset.o $(BUILDIR)/BackupServer.o
	$(CC) $(CFLAGS) $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/lazy.o $(BUILDIR)/coupling.o $(BUILDIR)/intset.o $(BUILDIR)/BackupServer.o -lpthread -o $(ROOT)/BackupServer $(LDFLAGS)

Client: $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/coupling.o $(BUILDIR)/lazy.o $(BUILDIR)/intset.o $(BUILDIR)/Client.o
	$(CC) $(CFLAGS) $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/lazy.o $(BUILDIR)/coupling.o $(BUILDIR)/intset.o $(BUILDIR)/Client.o -lpthread -o $(ROOT)/Client $(LDFLAGS)

$(BUILDIR)/linkedlist-lock.o: $(INCLUDE)/linkedlist-lock.h $(SRC)/linkedlist-lock.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-lock.o $(SRC)/linkedlist-lock.c -I$(INCLUDE)

$(BUILDIR)/lazy.o: $(INCLUDE)/lazy.h $(SRC)/lazy.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/lazy.o $(SRC)/lazy.c -I$(INCLUDE)

$(BUILDIR)/coupling.o: $(INCLUDE)/coupling.h $(SRC)/coupling.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/coupling.o $(SRC)/coupling.c -I$(INCLUDE)

$(BUILDIR)/intset.o: $(INCLUDE)/linkedlist-lock.h $(INCLUDE)/coupling.h $(INCLUDE)/lazy.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o $(SRC)/intset.c -I$(INCLUDE)

$(BUILDIR)/PrimaryServer.o: $(INCLUDE)/intset.h $(INCLUDE)/packet.h $(SRC)/PrimaryServer.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/PrimaryServer.o $(SRC)/PrimaryServer.c -I$(INCLUDE)

$(BUILDIR)/BackupServer.o: $(INCLUDE)/intset.h $(INCLUDE)/packet.h $(SRC)/BackupServer.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/BackupServer.o $(SRC)/BackupServer.c -I$(INCLUDE)

$(BUILDIR)/Client.o: $(INCLUDE)/intset.h $(INCLUDE)/packet.h $(SRC)/Client.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/Client.o $(SRC)/Client.c -I$(INCLUDE)

clean:
	rm -rf $(BINS) $(BUILDIR)/*
