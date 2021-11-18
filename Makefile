ROOT = .
TARGET  = PrimaryServer
BUILDIR = $(ROOT)/build
BINS = $(ROOT)/$(TARGET)
SRC = src
INCLUDE = include
DEBUG = Y

CFLAGS += -std=gnu89 -Wall
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY:	clean

$(TARGET): linkedlist-lock.o coupling.o lazy.o intset.o PrimaryServer.o
	$(CC) $(CFLAGS) $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/lazy.o $(BUILDIR)/coupling.o $(BUILDIR)/intset.o $(BUILDIR)/PrimaryServer.o -lpthread -o $(BINS) $(LDFLAGS)

linkedlist-lock.o: $(INCLUDE)/linkedlist-lock.h $(SRC)/linkedlist-lock.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-lock.o $(SRC)/linkedlist-lock.c -I$(INCLUDE)

lazy.o: $(INCLUDE)/lazy.h $(SRC)/lazy.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/lazy.o $(SRC)/lazy.c -I$(INCLUDE)

coupling.o: $(INCLUDE)/coupling.h $(SRC)/coupling.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/coupling.o $(SRC)/coupling.c -I$(INCLUDE)

intset.o: $(INCLUDE)/linkedlist-lock.h $(INCLUDE)/coupling.h $(INCLUDE)/lazy.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o $(SRC)/intset.c -I$(INCLUDE)

PrimaryServer.o: $(INCLUDE)/linkedlist-lock.h $(INCLUDE)/coupling.h $(INCLUDE)/lazy.h $(INCLUDE)/intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/PrimaryServer.o $(SRC)/PrimaryServer.c -I$(INCLUDE)

clean:
	rm -rf $(BINS) $(BUILDIR)/*
