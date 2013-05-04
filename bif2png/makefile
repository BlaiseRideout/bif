CC		= g++
CPPFLAGS 	:= -std=c++11 -Wall --pedantic -O2 -ggdb
LDFLAGS		= -lpng
BINNAME		= bif2png

.PHONY: clean install

$(BINNAME): $(BINNAME).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LDFLAGS)
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
