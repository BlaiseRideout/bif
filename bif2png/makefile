CC		= gcc
CPPFLAGS 	:= -Wall --std=c99 --pedantic -O2 -ggdb
BINNAME		= bif2png

.PHONY: clean install

$(BINNAME): $(BINNAME).c
	$(CC) $(CPPFLAGS) -o $@ $< $(GTKCFLAGS) $(GTKLIBS)
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
