CFLAGS ?= -O2
CPPFLAGS ?= -Iinclude -MMD

SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))

PREFIX ?= /usr/local

# Some systems only have ncursesw or ncurses
$(shell pkg-config --exists ncursesw)
ifeq ($(.SHELLSTATUS),0)
	NCURSES := ncursesw
else
	NCURSES := ncurses
endif

LIBS := $(shell pkg-config --libs $(NCURSES)) -ltinfo -lm
PKGCFLAGS := $(shell pkg-config --cflags $(NCURSES))

all: build c4c

build:
	mkdir $@

c4c: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PKGCFLAGS) -o $@ $^ $(LIBS)

install: all
	install -D c4c $(DESTDIR)$(PREFIX)/bin/c4c

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/c4c

clean:
	rm -f c4c $(OBJECTS) $(OBJECTS:.o=.d)
	rm -f -d build

-include  $(OBJECTS:.o=.d)
.PHONY: all install uninstall clean

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PKGCFLAGS) -c -o $@ $<

include/logo.h: res/logo
	scripts/genlogo.sh $< $@
