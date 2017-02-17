OBJS=popstation_md.o popstation.o
CFLAGS=-Wall -I.
LDFLAGS=-L.
LIBS = -lz

all: popstation_md popstation

popstation: popstation.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

popstation_md: popstation_md.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f popstation_md popstation *.o
