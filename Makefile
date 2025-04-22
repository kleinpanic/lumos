PREFIX     ?= /usr/local
BINDIR     := $(PREFIX)/bin
INCLUDEDIR := $(PREFIX)/include
MANPREFIX  := $(PREFIX)/share/man

CC       ?= gcc
CFLAGS   ?= -std=c11 -Wall -Wextra -Iinclude $(shell pkg-config --cflags x11)
LDFLAGS  ?= $(shell pkg-config --libs x11) -lm -li2c

SRC      := src/main.c src/brightness.c
TARGET   := lumos
MANPAGE  := $(wildcard docs/*.1)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -m 644 include/brightness.h $(DESTDIR)$(INCLUDEDIR)/
ifneq ($(MANPAGE),)
	install -d $(DESTDIR)$(MANPREFIX)/man1
	install -m 644 $(MANPAGE) $(DESTDIR)$(MANPREFIX)/man1/$(notdir $(MANPAGE))
endif

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(INCLUDEDIR)/brightness.h
ifneq ($(MANPAGE),)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(notdir $(MANPAGE))
endif

