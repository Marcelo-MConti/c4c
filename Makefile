CFLAGS ?= -O2
CPPFLAGS ?= -Iinclude -MMD

SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))

# Some systems only have ncursesw or ncurses
$(shell pkg-config --exists ncursesw)
ifeq ($(.SHELLSTATUS),0)
	NCURSES := ncursesw
else
	NCURSES := ncurses
endif

LIBS += $(shell pkg-config --libs $(NCURSES)) -lm -ltinfo
override CFLAGS := $(CFLAGS) $(shell pkg-config --cflags $(NCURSES))

PREFIX ?= /usr/local


all: build c4c

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f c4c $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/c4c

clean:
	rm -f c4c $(OBJECTS) $(OBJECTS:.o=.d)
	rm -d -f build

.PHONY: all install uninstall clean

-include  $(OBJECTS:.o=.d)


build:
	mkdir -p build

c4c: $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

include/logo.h: res/logo
	scripts/genlogo.sh $< $@
