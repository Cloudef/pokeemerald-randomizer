#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <err.h>

#include "pokemon.h"
extern int sha1digest(uint8_t *digest, char *hexdigest, const uint8_t *data, size_t databytes);
#include "bps.h"
#include "data.h"

struct PMappedRom P_MAPPED_ROM;

static void* LOAD(ptrdiff_t addr) {
   return (char*)P_MAPPED_ROM.base + (addr - ROM_LOAD_ADDRESS);
}

static ptrdiff_t DEREF(ptrdiff_t addr) {
   uint32_t u32ptr;
   memcpy(&u32ptr, LOAD(addr), sizeof(u32ptr));
   return u32ptr;
}

struct PMapHeader* pokemon_map_header_by_group_and_id(const uint8_t group, const uint8_t id) {
   return LOAD(DEREF(DEREF(ROM_MAP_GROUPS_ADDRESS + group * 4) + id * 4));
}

uint16_t pokemon_map_index_from_group_and_id(const uint8_t group, const uint8_t id) {
   struct PMapHeader *hdr = pokemon_map_header_by_group_and_id(group, id);
   return ((char*)hdr - (char*)P_MAPPED_ROM.maps) / sizeof(*hdr);
}

struct PMapEvents* pokemon_map_header_events(struct PMapHeader *header) { return LOAD(header->events); }

struct PWarpEvent* pokemon_map_events_warps(struct PMapEvents *events, uint8_t *out_num) {
   if (out_num) *out_num = events->warpCount;
   return LOAD(events->warps);
}

void pokemon_map_rom(const char *path) {
   int fd;
   if ((fd = open(path, O_RDONLY)) < 0)
      err(EXIT_FAILURE, "open(%s, O_RDONLY)", path);

   struct stat s;
   if (fstat(fd, &s) < 0)
      err(EXIT_FAILURE, "fstat");

   void *mapped;
   if (!(mapped = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)))
      err(EXIT_FAILURE, "mmap failed");

   {
      uint8_t digest[20];
      char hash[sizeof(digest)*2+1];
      sha1digest(digest, hash, mapped, s.st_size);

      if (memcmp(ORIGINAL_ROM_SHA1, hash, sizeof(ORIGINAL_ROM_SHA1)) != 0)
         errx(EXIT_FAILURE, "rom is not original pokemon emerald rom (hash: %s, expected: %s)", hash, ORIGINAL_ROM_SHA1);
   }

   void *rom;
   if (!(rom = malloc(s.st_size)))
      err(EXIT_FAILURE, "malloc");

   if (!bps_patch(rom, s.st_size, mapped, s.st_size, bps_patch_bin, bps_patch_bin_len))
      errx(EXIT_FAILURE, "bps_patch failed");

   {
      uint8_t digest[20];
      char hash[sizeof(digest)*2+1];
      sha1digest(digest, hash, rom, s.st_size);

      if (memcmp(PATCHED_ROM_SHA1, hash, sizeof(PATCHED_ROM_SHA1)) != 0)
         errx(EXIT_FAILURE, "patching produced and unexpected result (hash: %s, expected: %s)", hash, PATCHED_ROM_SHA1);
   }

   munmap(mapped, s.st_size);
   close(fd);

   P_MAPPED_ROM.base = rom; // for early DEREF
   P_MAPPED_ROM = (struct PMappedRom){
      .base = rom,
      .size = s.st_size,
      .maps = pokemon_map_header_by_group_and_id(0, 0),
      .map_names = ROM_MAP_NAMES,
      .map_count = sizeof(ROM_MAP_NAMES) / sizeof(*ROM_MAP_NAMES),
   };
}

void pokemon_unmap_rom() {
   free(P_MAPPED_ROM.base);
   P_MAPPED_ROM = (struct PMappedRom){0};
}
