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

SRC = src/pokemon.h src/pokemon_text.h src/filter.h src/bps.h src/data.h src/blacklist.h \
      src/randomizer.c src/pokemon.c src/pokemon_text.c src/filter.c src/teeny-sha1.c src/tinymt32.c src/bps.c

all: randomizer

%.a:
	$(LINK.c) -c $(filter %.c,$^) -o $@

patched.sha1:
	./bootstrap-hack.bash sha1 > $@

src/data.h: patched.sha1 original.sha1
	./bootstrap-hack.bash

src/blacklist.h: blacklist.default
	xxd -i $< > $@

randomizer: $(SRC)
	$(LINK.c) $(filter %.c %.a,$^) $(LDLIBS) -o $@

wasi-impl.a: src/wasi.c
randomizer.wasm: $(SRC) wasi-impl.a
	$(LINK.c) -D_WASI_EMULATED_MMAN $(filter %.c %.a,$^) $(LDLIBS) -lwasi-emulated-mman -o $@

install:
	test -f randomizer && install -Dm755 randomizer $(DESTDIR)$(PREFIX)$(bindir)/pokeemerald-randomizer || test -f randomizer.wasm
	test -f randomizer.wasm && install -Dm644 randomizer.wasm $(DESTDIR)$(PREFIX)$(bindir)/pokeemerald-randomizer.wasm || test -f randomizer

clean:
	rm -f randomizer
	rm -rf randomizer.dSYM

.DELETE_ON_ERROR:
.PHONY: all clean install
