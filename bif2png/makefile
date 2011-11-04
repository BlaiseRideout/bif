CC		= gcc
CPPFLAGS 	:= -Wall --std=c99 --pedantic -O2 -ggdb
LDFLAGS		= -lpng
BINNAME		= bif2png

.PHONY: clean install

$(BINNAME): $(BINNAME).c
	$(CC) $(CPPFLAGS) -o $@ $< $(LDFLAGS)
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
