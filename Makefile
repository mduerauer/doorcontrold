PROJECT=doorcontrold
SOURCES=src/doorcontrold.c
INCPATHS=/usr/local/include
LIBPATHS=/usr/local/lib
LDFLAGS=
CFLAGS=-Wall
CC=gcc

INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

BINARY=$(PROJECT)

all: $(BINARY)

$(BINARY):
	$(CC) -o $(PROJECT) $(SOURCES) $(CFLAGS) -Isrc/ $(INCFLAGS) $(LIBFLAGS) -lpifacedigital -lmcp23s17

distclean: clean

clean:
	rm -f $(BINARY)

install: $(BINARY)
	install $(BINARY) /usr/local/bin
