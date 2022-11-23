#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <memory.h>
#include <stdarg.h>
#include <err.h>

#include "tinymt32.h"
#include "pokemon.h"
#include "filter.h"
#include "blacklist.h"

static struct {
   struct filter *filter;
   uint32_t seed;
   bool linked_warps;
   bool resolved_only;
   bool use_warp_filter;
   bool randomize_mons;
   bool randomize_trainers;
   bool verbose;
} ARGS = {
   .linked_warps = true,
   .resolved_only = true,
   .use_warp_filter = true,
   .randomize_mons = false,
   .randomize_trainers = false,
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

static bool should_skip_warp(const uint16_t map_index, const uint8_t warp_id, struct PWarpEvent *warp) {
   if (warp->warpId == WARP_ID_NONE || warp->warpId == WARP_ID_DYNAMIC || warp->warpId == WARP_ID_SECRET_BASE ||
      (warp->mapNum == MAP_NUM(DYNAMIC) && warp->mapGroup == MAP_GROUP(DYNAMIC)) || (warp->mapNum == MAP_NUM(UNDEFINED) && warp->mapGroup == MAP_GROUP(UNDEFINED)))
      return true;

   {
      const char *name = P_MAPPED_ROM.map_names[map_index];
      switch (filter_policy_for_input(ARGS.filter,  P_MAPPED_ROM.map_names[map_index])) {
         case ALL:
         case IN:
            verbose("filter:  IN %s[%hhu]", name, warp_id);
            return true;
         case OUT:
            {
               // TODO: ugly
               struct PWarpEvent dst = pokemon_map_header_warps(&P_MAPPED_ROM.maps[map_index], NULL)[warp_id];
               *warp = dst;
               break;
            }
         case ALLOW:
            break;
      }
   }

   {
      const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warp->mapGroup, warp->mapNum);
      const char *name = P_MAPPED_ROM.map_names[dst_map_index];
      switch (filter_policy_for_input(ARGS.filter, name)) {
         case ALL:
         case OUT:
            verbose("filter: OUT %s[%hhu]", name, warp_id);
            return true;
         case IN:
            {
               // TODO: ugly
               struct PWarpEvent dst = pokemon_map_header_warps(&P_MAPPED_ROM.maps[dst_map_index], NULL)[warp->warpId];
               *warp = dst;
               break;
            }
         case ALLOW:
            break;
      }
   }

   return false;
}

static size_t iterate_warps(void (*cb)(const uint16_t map_index, const size_t warp_index, const uint8_t warp_id, struct PWarpEvent *warp, void *uptr), void *uptr) {
   size_t warp_index = 0;
   for (uint16_t i = 0; i < P_MAPPED_ROM.map_count; ++i) {
      uint8_t num_warps;
      struct PWarpEvent *warps = pokemon_map_header_warps(&P_MAPPED_ROM.maps[i], &num_warps);
      for (uint8_t k = 0; k < num_warps; ++k) {
         if (should_skip_warp(i, k, &warps[k])) continue;
         if (cb) cb(i, warp_index, k, &warps[k], uptr);
         ++warp_index;
      }
   }
   return warp_index;
}

static void collect_resolved_warps(const uint16_t map_index, const size_t warp_index, const uint8_t warp_id, struct PWarpEvent *warp, void *uptr) {
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
      bool resolved = false;
      for (size_t k = i; k < n; ++k) {
         const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warps[k].data.mapGroup, warps[k].data.mapNum);
         if (dst_map_index != warps[i].meta.srcMapIndex || warps[k].data.warpId != warps[i].meta.srcWarpId) continue;
         swap_resolved_warp(&warps[k], &warps[++i]);
         *out_num_linked_warps += 2;
         resolved = true;
         break;
      }
      if (!resolved) {
         const uint16_t dst_map_index = pokemon_map_index_from_group_and_id(warps[i].data.mapGroup, warps[i].data.mapNum);
         verbose("unresolved: %s[%02hhu] <=x %s[%02hhu]",
               P_MAPPED_ROM.map_names[warps[i].meta.srcMapIndex], warps[i].meta.srcWarpId,
               P_MAPPED_ROM.map_names[dst_map_index], warps[i].data.warpId);
         if (n > 0) swap_resolved_warp(&warps[i], &warps[n - 1]);
         --i, --n;
      }
   }

   if (!ARGS.resolved_only)
      n = orig_n;

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

   for (; i + 1 < n; ++i) {
      const size_t rnd = tinymt32_generate_uint32(prng);
      const size_t r = (rnd % (n - i - 1) + i + 1);
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

static uint16_t random_species(tinymt32_t *prng) {
   return 1 + (tinymt32_generate_uint32(prng) % (NUM_SPECIES - 2));
}

static void usage(FILE *out, const char *name)
{
    char *base = strrchr(name, '/');
    fprintf(out, "usage: %s [options]\n", (base ? base + 1 : name));
    fputs("Options\n"
          " -h, --help            display this help and exit.\n"
          " -d, --verbose         verbose output.\n"
          " -o, --output          specify output path.\n"
          " -s, --seed            specify seed manually.\n"
          " -f, --filter          provide custom warp blacklist.\n"
          " --unlinked-warps      entrance and exit may be different.\n"
          " --allow-unresolved    include unpaired warps (softlock likely).\n"
          " --no-filter           do not use the default warp blacklist.\n"
          " --randomize-mons      randomize wild encounters.\n"
          " --randomize-trainers  randomize trainer battles.\n"
          , out);

    exit((out == stderr ? EXIT_FAILURE : EXIT_SUCCESS));
}

int main(int argc, char * const argv[]) {
   const char *output_path = NULL;
   {
      static struct option opts[] = {
         { "help",               no_argument,       0,  'h'    },
         { "verbose",            no_argument,       0,  'd'    },
         { "output",             required_argument, 0,  'o'    },
         { "seed",               required_argument, 0,  's'    },
         { "filter",             required_argument, 0,  'f'    },
         { "unlinked-warps",     no_argument,       0,  0x1000 },
         { "allow-unresolved",   no_argument,       0,  0x1001 },
         { "no-filter",          no_argument,       0,  0x1002 },
         { "randomize-mons",     no_argument,       0,  0x1003 },
         { "randomize-trainers", no_argument,       0,  0x1004 },
         { 0,                    0,                 0,  0      }
      };

      bool got_seed = false;
      const char *filter_path = NULL;
      for (optind = 0;;) {
         int32_t opt;
         if ((opt = getopt_long(argc, argv, "hds:o:f:", opts, NULL)) < 0)
            break;

         switch (opt) {
            case 'h':
               usage(stdout, argv[0]);
               break;
            case 'd':
               ARGS.verbose = true;
               break;

            case 'o':
               output_path = optarg;
               break;
            case 's':
               ARGS.seed = strtoul(optarg, NULL, 10);
               got_seed = true;
               break;
            case 'f':
               filter_path = optarg;
               break;

            case 0x1000:
               ARGS.linked_warps = false;
               break;
            case 0x1001:
               ARGS.resolved_only = false;
               break;
            case 0x1002:
               ARGS.use_warp_filter = false;
               break;
            case 0x1003:
               ARGS.randomize_mons = true;
               break;
            case 0x1004:
               ARGS.randomize_trainers = true;
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

      if (ARGS.use_warp_filter) {
         if (filter_path) {
            ARGS.filter = filter_load(filter_path);
         } else {
            ARGS.filter = filter_load_mem(blacklist_default, blacklist_default_len);
         }
      }
   }

   if (argc <= 1)
      usage(stderr, argv[0]);

   tinymt32_t prng;
   tinymt32_init(&prng, ARGS.seed);
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
      shuffle_resolved_warps(warps, num_linked_warps, total_warps, &prng);
      apply_resolved_warps(warps, total_warps);
      free(warps);
   }

   if (ARGS.randomize_mons) {
      warnx("randomizing mons ...");

      if (P_MAPPED_ROM.starters) {
         for (uint8_t i = 0; i < 3; ++i) P_MAPPED_ROM.starters->species[i] = random_species(&prng);
      }

      for (size_t i = 0; i < P_MAPPED_ROM.mon_count; ++i) {
         for (uint8_t t = 0; t < INFO_COUNT; ++t) {
            if (!P_MAPPED_ROM.mons[i].info[t]) continue;
            struct PWildPokemonInfo *info = pokemon_load(P_MAPPED_ROM.mons[i].info[t]);
            struct PWildPokemon *wild = pokemon_load(info->wildPokemon);
            for (int w = 0; w < WILD_COUNT_FOR[t]; ++w) wild[w].species = random_species(&prng);
         }
      }
   }

   {
      char out_name[1024];
      if (!output_path) {
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
         output_path = out_name;
      }

      FILE *f;
      if (!(f = fopen(output_path, "wb")))
         err(EXIT_FAILURE, "fopen(%s, wb)", output_path);

      fwrite(P_MAPPED_ROM.base, 1, P_MAPPED_ROM.size, f);
      fclose(f);

      const bool prefix = (output_path[0] != '/' && strncmp("./", output_path, 2));
      warnx("modified ROM at path %s%s", (prefix ? "./" : ""), output_path);
   }

   return EXIT_SUCCESS;
}
