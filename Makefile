BUILD ?= build

SOURCES := $(wildcard src/*.c src/game/*.c)
OBJECTS := $(SOURCES:src/%.c=$(BUILD)/%.o)

PREFIX ?= /usr/local

# Some systems only have ncursesw or ncurses
$(shell pkg-config --exists ncursesw)
ifeq ($(.SHELLSTATUS),0)
	NCURSES := ncursesw
else
	NCURSES := ncurses
endif

ifneq ($(origin ASCII), undefined)
	CPPFLAGS += -DC4C_ASCII
endif

ifeq ($(origin NOCOLOR), undefined)
	CPPFLAGS += -DC4C_COLOR
endif

ALLCFLAGS := -Wall -O2 -std=gnu11 $(CFLAGS)
ALLCPPFLAGS := -Iinclude -MMD $(CPPFLAGS)

LIBS := $(shell pkg-config --libs $(NCURSES)) -ltinfo -lm
PKGCFLAGS := $(shell pkg-config --cflags $(NCURSES))

all: $(dir $(OBJECTS)) c4c

%/:
	mkdir $@

c4c: $(OBJECTS)
	$(CC) $(ALLCPPFLAGS) $(ALLCFLAGS) $(PKGCFLAGS) -o $@ $^ $(LIBS)

install: all
	install -D c4c $(DESTDIR)$(PREFIX)/bin/c4c

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/c4c

clean:
	rm -f c4c $(OBJECTS) $(OBJECTS:.o=.d)
	rm -f -d $(BUILD)

-include  $(OBJECTS:.o=.d)
.PHONY: all install uninstall clean

$(BUILD)/%.o: src/%.c
	$(CC) $(ALLCPPFLAGS) $(ALLCFLAGS) $(PKGCFLAGS) -c -o $@ $<
