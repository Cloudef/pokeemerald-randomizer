#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <memory.h>
#include <stdarg.h>
#include <err.h>

#include "tinymt32.h"
#include "pokemon.h"

static struct {
   uint32_t seed;
   bool linked_warps;
   bool resolved_only;
   bool verbose;
} ARGS = {
   .linked_warps = true,
   .resolved_only = true,
   .verbose = false,
};

struct ResolvedWarp {
   struct ResolvedWarpData {
      uint8_t warpId, mapNum, mapGroup;
   } data;
   struct ResolvedWarpMeta {
      uint16_t srcMapIndex;
      uint8_t srcWarpId;
   } meta;
};

__attribute__ ((format(printf, 1, 2)))
static inline void verbose(const char *fmt, ...) {
   if (!ARGS.verbose) return;
   va_list args;
   va_start(args, fmt);
   vprintf(fmt, args);
   va_end(args);
   putc('\n', stdout);
}

static bool should_skip_warp(struct PWarpEvent *warp) {
   return warp->warpId == WARP_ID_NONE || warp->warpId == WARP_ID_DYNAMIC || warp->warpId == WARP_ID_SECRET_BASE ||
         (warp->mapNum == MAP_NUM(DYNAMIC) && warp->mapGroup == MAP_GROUP(DYNAMIC)) ||
         (warp->mapNum == MAP_NUM(UNDEFINED) && warp->mapGroup == MAP_GROUP(UNDEFINED));
}

static size_t iterate_warps(void (*cb)(const size_t map_index, const size_t warp_index, const uint8_t warp_id, struct PWarpEvent *warp, void *uptr), void *uptr) {
   size_t warp_index = 0;
   for (uint16_t i = 0; i < P_MAPPED_ROM.map_count; ++i) {
      uint8_t num_warps;
      struct PWarpEvent *warps = pokemon_map_header_warps(&P_MAPPED_ROM.maps[i], &num_warps);
      for (uint8_t k = 0; k < num_warps; ++k) {
         if (should_skip_warp(&warps[k])) continue;
         if (cb) cb(i, warp_index, k, &warps[k], uptr);
         ++warp_index;
      }
   }
   return warp_index;
}

static void collect_resolved_warps(const size_t map_index, const size_t warp_index, const uint8_t warp_id, struct PWarpEvent *warp, void *uptr) {
   struct ResolvedWarp *warps = uptr;
   warps[warp_index] = (struct ResolvedWarp){
      .meta.srcWarpId = warp_id,
      .meta.srcMapIndex = map_index,
      .data.warpId = warp->warpId,
      .data.mapNum = warp->mapNum,
      .data.mapGroup = warp->mapGroup
   };
}

static void swap_resolved_warp(struct ResolvedWarp *a, struct ResolvedWarp *b) {
   const struct ResolvedWarp tmp = *a;
   *a = *b;
   *b = tmp;
}

static size_t resolve_warp_links(struct ResolvedWarp *warps, size_t n, size_t *out_num_linked_warps) {
   const size_t orig_n = n;

   *out_num_linked_warps = 0;
   for (size_t i = 0; n > 0 && i < n - 1; ++i) {
      size_t resolved = 0, resolved_at = 0;
      for (size_t k = i; k < n; ++k) {
         const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warps[k].data.mapGroup, warps[k].data.mapNum);
         if (dst_map_index != warps[i].meta.srcMapIndex || warps[k].data.warpId != warps[i].meta.srcWarpId) continue;
         resolved_at = k;
         ++resolved;
      }

      if (resolved == 1) {
         swap_resolved_warp(&warps[resolved_at], &warps[++i]);
         *out_num_linked_warps += 2;
         continue;
      } else if (resolved > 0) {
         verbose("warning: %s[%02hhu] has %zu different connections", P_MAPPED_ROM.map_names[warps[i].meta.srcMapIndex], warps[i].meta.srcWarpId, resolved);
      } else {
         const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warps[i].data.mapGroup, warps[i].data.mapNum);
         verbose("unresolved: %s[%02hhu] <=x %s[%02hhu]",
               P_MAPPED_ROM.map_names[warps[i].meta.srcMapIndex], warps[i].meta.srcWarpId,
               P_MAPPED_ROM.map_names[dst_map_index], warps[i].data.warpId);
      }

      if (n > 0) swap_resolved_warp(&warps[i], &warps[n - 1]);
      --i, --n;
   }

   if (!ARGS.resolved_only) n = orig_n;

   if (orig_n != n) {
      warnx("found %zu warps from which %zu could not be resolved (use --verbose to show them)", orig_n, orig_n - n);
   } else {
      warnx("found %zu warps", n);
   }

   if (*out_num_linked_warps % 2 != 0)
      errx(EXIT_FAILURE, "amount of linked warps %zu is not divisible by 2, this shouldn't be possible...?", *out_num_linked_warps);

   return n;
}

static void swap_resolved_warp_data(struct ResolvedWarp *a, struct ResolvedWarp *b) {
   const struct ResolvedWarpData tmp = a->data;
   a->data = b->data;
   b->data = tmp;
}

static void swap_resolved_warp_meta(struct ResolvedWarp *a, struct ResolvedWarp *b) {
   const struct ResolvedWarpMeta tmp = a->meta;
   a->meta = b->meta;
   b->meta = tmp;
}

static void shuffle_resolved_warps(struct ResolvedWarp *warps, const size_t linked_n, const size_t n, tinymt32_t *prng) {
   size_t i = 0;
   if (ARGS.linked_warps) {
      for (; i + 2 < linked_n; i += 2) {
         const size_t rnd = tinymt32_generate_uint32(prng) * 2;
         const size_t r = (rnd % (linked_n - i - 2)) + i + 2;
         if (!!(tinymt32_generate_uint32(prng) % 2)) {
            swap_resolved_warp_data(&warps[i], &warps[r]);
            swap_resolved_warp_meta(&warps[i + 1], &warps[r + 1]);
         } else {
            swap_resolved_warp_meta(&warps[i], &warps[r]);
            swap_resolved_warp_data(&warps[i + 1], &warps[r + 1]);
         }
      }
   }

   for (; n > 0 && i < n - 1; ++i) {
      const size_t rnd = tinymt32_generate_uint32(prng);
      const size_t r = i + (rnd % (n - i) + 1);
      swap_resolved_warp_data(&warps[i], &warps[r]);
   }
}

static void apply_resolved_warps(const struct ResolvedWarp *warps, const size_t n) {
   for (size_t i = 0; i < n; ++i) {
      const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warps[i].data.mapGroup, warps[i].data.mapNum);
      verbose("linking: %s[%02hhu] <=> %s[%02hhu]",
            P_MAPPED_ROM.map_names[warps[i].meta.srcMapIndex], warps[i].meta.srcWarpId,
            P_MAPPED_ROM.map_names[dst_map_index], warps[i].data.warpId);
      struct PWarpEvent *warp = &pokemon_map_header_warps(&P_MAPPED_ROM.maps[warps[i].meta.srcMapIndex], NULL)[warps[i].meta.srcWarpId];
      warp->warpId = warps[i].data.warpId;
      warp->mapNum = warps[i].data.mapNum;
      warp->mapGroup = warps[i].data.mapGroup;
   }
}

static void
usage(FILE *out, const char *name)
{
    char *base = strrchr(name, '/');
    fprintf(out, "usage: %s [options]\n", (base ? base + 1 : name));
    fputs("Options\n"
          " -h, --help            display this help and exit.\n"
          " -d, --verbose         verbose output.\n"
          " -s, --seed            specify seed manually.\n"
          " --unlinked-warps      entrance and exit may be different.\n"
          " --allow-unresolved    include unpaired warps (softlock likely).\n"
          , out);

    exit((out == stderr ? EXIT_FAILURE : EXIT_SUCCESS));
}

int main(int argc, char * const argv[]) {
   {
      static struct option opts[] = {
         { "help",             no_argument,       0,  'h'    },
         { "verbose",          no_argument,       0,  'd'    },
         { "seed",             required_argument, 0,  's'    },
         { "unlinked-warps",   no_argument,       0,  0x1000 },
         { "allow-unresolved", no_argument,       0,  0x1001 },
         { 0,                  0,                 0,  0      }
      };

      bool got_seed = false;
      for (optind = 0;;) {
         int32_t opt;
         if ((opt = getopt_long(argc, argv, "hs:", opts, NULL)) < 0)
            break;

         switch (opt) {
            case 'h':
               usage(stdout, argv[0]);
               break;
            case 'd':
               ARGS.verbose = true;
               break;

            case 's':
               ARGS.seed = strtoul(optarg, NULL, 10);
               got_seed = true;
               break;

            case 0x1000:
               ARGS.linked_warps = false;
               break;
            case 0x1001:
               ARGS.resolved_only = false;
               break;

            case ':':
            case '?':
               fputs("\n", stderr);
               usage(stderr, argv[0]);
               break;
         }
      }

      if (!got_seed) ARGS.seed = arc4random();

      if (optind > 0) {
         argc -= (optind - 1);
         argv += (optind - 1);
      }
   }

   if (argc <= 1)
      errx(EXIT_FAILURE, "must provide a path to the rom");

   warnx("seed %u", ARGS.seed);
   pokemon_map_rom(argv[1]);

   {
      size_t total_warps = iterate_warps(NULL, NULL);

      struct ResolvedWarp *warps;
      if (!(warps = calloc(sizeof(struct ResolvedWarp), total_warps)))
         err(EXIT_FAILURE, "calloc(%zu, %zu)", sizeof(struct ResolvedWarp), total_warps);

      iterate_warps(collect_resolved_warps, warps);

      size_t num_linked_warps;
      total_warps = resolve_warp_links(warps, total_warps, &num_linked_warps);

      tinymt32_t prng;
      tinymt32_init(&prng, ARGS.seed);
      shuffle_resolved_warps(warps, num_linked_warps, total_warps, &prng);

      apply_resolved_warps(warps, num_linked_warps);
      free(warps);
   }

   {
      char out_name[1024];
      char *slash = strrchr(argv[1], '/');
      if (slash) {
         const char *dir = argv[1];
         *slash = 0;
         const char *base = slash + 1;
         snprintf(out_name, sizeof(out_name), "%s/%u-%s", dir, ARGS.seed, base);
         *slash = '/';
      } else {
         snprintf(out_name, sizeof(out_name), "%u-%s", ARGS.seed, argv[1]);
      }

      FILE *f;
      if (!(f = fopen(out_name, "wb")))
         err(EXIT_FAILURE, "fopen(%s, wb)", out_name);

      fwrite(P_MAPPED_ROM.base, 1, P_MAPPED_ROM.size, f);
      fclose(f);

      const bool prefix = (out_name[0] != '/' && strncmp("./", out_name, 2));
      warnx("modified ROM at path %s%s", (prefix ? "./" : ""), out_name);
   }

   return EXIT_SUCCESS;
}
