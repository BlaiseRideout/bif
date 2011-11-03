CC		= gcc
CPPFLAGS 	:= -Wall --std=c99 --pedantic -O2 -ggdb
GTKCFLAGS	= `pkg-config --cflags gtk+-2.0`
GTKLIBS 	= `pkg-config --libs gtk+-2.0`
BINNAME		= guiimg

.PHONY: clean install

$(BINNAME): $(BINNAME).c
	$(CC) $(CPPFLAGS) -o $@ $< $(GTKCFLAGS) $(GTKLIBS)
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
