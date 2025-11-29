BUILD ?= build

SOURCES := $(wildcard src/*.c src/game/*.c src/ui/*.c)
OBJECTS := $(SOURCES:src/%.c=$(BUILD)/%.o)

PREFIX ?= /usr/local

ALLCFLAGS := -Wall -O2 -std=gnu11
ALLCPPFLAGS := -Iinclude -MMD -DPREFIX=\"$(PREFIX)\"

# Some systems only have ncursesw or ncurses
$(shell pkg-config --exists ncursesw)
ifeq ($(.SHELLSTATUS),0)
	NCURSES := ncursesw
else
	NCURSES := ncurses
endif

ifneq ($(origin ASCII), undefined)
	ALLCPPFLAGS += -DC4C_ASCII
endif

ifeq ($(origin NOCOLOR), undefined)
	ALLCPPFLAGS += -DC4C_COLOR
endif

ALLCFLAGS += $(CFLAGS)
ALLCPPFLAGS += $(CPPFLAGS)

LIBS := $(shell pkg-config --libs $(NCURSES)) -ltinfo -lm -lpthread $(LDLIBS)
PKGCFLAGS := $(shell pkg-config --cflags $(NCURSES))

LANGS := pt
MOBJECTS := $(LANGS:%=$(BUILD)/messages/%/LC_MESSAGES/c4c.mo)

all: c4c mo

%/:
	mkdir -p $@

c4c: $(dir $(OBJECTS)) $(OBJECTS)
	$(CC) $(ALLCPPFLAGS) $(ALLCFLAGS) $(PKGCFLAGS) -o $@ $(OBJECTS) $(LIBS)

install: c4c $(MOBJECTS)
	install -D c4c $(DESTDIR)$(PREFIX)/bin/c4c
	mkdir -p $(DESTDIR)$(PREFIX)/share/locale
	cp -r $(LANGS:%=$(BUILD)/messages/%) $(DESTDIR)$(PREFIX)/share/locale

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/c4c
	rm -f $(LANGS:%=$(DESTDIR)$(PREFIX)/share/locale/%/LC_MESSAGES/c4c.mo)

clean:
	rm -rf $(BUILD)

mo: $(MOBJECTS)

-include  $(OBJECTS:.o=.d)
.PHONY: all install uninstall clean mo

$(BUILD)/%.o: src/%.c
	$(CC) $(ALLCPPFLAGS) $(ALLCFLAGS) $(PKGCFLAGS) -c -o $@ $<

messages/all.pot: $(SOURCES)
	xgettext --no-location -j -o $@ -k_ -kN_ $(SOURCES)
	grep -v 'POT-Creation-Date:' $@ > $@.nostamp
	mv $@.nostamp $@

messages/%.po: messages/all.pot
	msgmerge -U $@ $?
	touch $@

$(BUILD)/messages/%/LC_MESSAGES/c4c.mo: messages/%.po $(BUILD)/messages/%/LC_MESSAGES/
	msgfmt -o $@ $<
	touch $@

.PRECIOUS: %/
