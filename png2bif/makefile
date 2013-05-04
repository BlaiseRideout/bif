CC		= g++
CPPFLAGS 	:= -Wall --std=c++11 --pedantic -O2 -ggdb
LDFLAGS		= -lpng
BINNAME		= png2bif

.PHONY: clean install

$(BINNAME): $(BINNAME).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LDFLAGS)
	
clean:
	rm -vf $(BINNAME)

install:
	cp $(BINNAME) /usr/bin/$(BINNAME)
