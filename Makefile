OBJS=popstation_md.o popstation.o common.o
CFLAGS=-Wall -I.
LDFLAGS=-L.
LIBS = -lz

all: popstation_md popstation

popstation_md: popstation_md.o common.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

popstation: popstation.o common.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS) -liniparser

clean:
	rm -f popstation_md popstation *.o
