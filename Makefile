OBJS=popstation_toc.o popstation_md.o popstation.o
CFLAGS=-Wall -I.
LDFLAGS=-L.
LIBS = -lz

all: popstation_toc popstation_md popstation

popstation_toc: popstation_toc.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS) -liniparser

popstation_md: popstation_md.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

popstation: popstation.o
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f popstation_toc popstation_md popstation *.o
