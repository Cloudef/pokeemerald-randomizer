PREFIX ?= /usr/local
bindir ?= /bin

MAKEFLAGS += --no-builtin-rules

WARNINGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=3 -Wstrict-overflow=5 -Wstack-usage=12500 \
	   -Wcast-align -Wpointer-arith -Wchar-subscripts -Warray-bounds=2 -Wno-unknown-warning-option

VISIBILITY ?= -fvisibility=hidden
override CFLAGS ?= -g -O2 $(WARNINGS) $(EXTRA_CFLAGS)
override CFLAGS += -std=c99 $(VISIBILITY)
override CPPFLAGS ?= -D_FORTIFY_SOURCE=2 $(EXTRA_CPPFLAGS)
override CPPFLAGS += -D_DEFAULT_SOURCE

all: randomizer

patched.sha1:
	./bootstrap-hack.bash sha1 > $@

src/data.h: patched.sha1 original.sha1
	./bootstrap-hack.bash

src/blacklist.h: blacklist.default
	xxd -i $< > $@

randomizer: src/pokemon.h src/pokemon_text.h src/filter.h src/bps.h src/data.h src/blacklist.h
randomizer: src/randomizer.c src/pokemon.c src/pokemon_text.c src/filter.c src/teeny-sha1.c src/tinymt32.c src/bps.c
	$(LINK.c) $(filter %.c,$^) $(LDLIBS) -o $@

install:
	install -Dm755 randomizer $(DESTDIR)$(PREFIX)$(bindir)/pokeemerald-randomizer

clean:
	rm -f randomizer
	rm -rf randomizer.dSYM

.DELETE_ON_ERROR:
.PHONY: all clean install
