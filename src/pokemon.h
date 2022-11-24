#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static const uint16_t MAP_DYNAMIC = (0x7F | (0x7F << 8));
static const uint16_t MAP_UNDEFINED = (0xFF | (0xFF << 8));
static const uint8_t WARP_ID_SECRET_BASE = 0x7E;
static const uint8_t WARP_ID_DYNAMIC = 0x7F;
static const uint8_t WARP_ID_NONE = -1;

#define MAP_GROUP(map) (MAP_##map >> 8)
#define MAP_NUM(map) (MAP_##map & 0xFF)

#define SPECIES_SHINY_TAG 500
#define MAX_TRAINER_ITEMS 4
#define TRAINER_NAME_LENGTH 10
#define MAX_MON_MOVES 4
#define NUM_SPECIES 412

#define LAND_WILD_COUNT 12
#define WATER_WILD_COUNT 5
#define ROCK_WILD_COUNT 5
#define FISH_WILD_COUNT 10

struct PMapHeader {
   uint32_t mapLayout;
   uint32_t events;
   uint32_t mapScripts;
   uint32_t connections;
   uint16_t music;
   uint16_t mapLayoutId;
   uint8_t regionMapSectionId;
   uint8_t cave;
   uint8_t weather;
   uint8_t mapType;
   uint8_t filler_18[2];
   bool allowCycling:1;
   bool allowEscaping:1;
   bool allowRunning:1;
   bool showMapName:1;
   uint8_t unused:4;
   uint8_t battleType;
};

struct PMapEvents {
   uint8_t objectEventCount;
   uint8_t warpCount;
   uint8_t coordEventCount;
   uint8_t bgEventCount;
   uint32_t objectEvents;
   uint32_t warps;
   uint32_t coordEvents;
   uint32_t bgEvents;
};

struct PWarpEvent {
   int16_t x, y;
   uint8_t elevation, warpId, mapNum, mapGroup;
};

struct PWildPokemon {
   uint8_t minLevel, maxLevel;
   uint16_t species;
};

struct PWildPokemonInfo {
   uint8_t encounterRate;
   uint32_t wildPokemon;
};

__attribute__((unused))
static uint8_t WILD_COUNT_FOR[] = {
   LAND_WILD_COUNT,
   WATER_WILD_COUNT,
   ROCK_WILD_COUNT,
   FISH_WILD_COUNT
};

enum {
   INFO_LAND,
   INFO_WATER,
   INFO_ROCK,
   INFO_FISH,
   INFO_COUNT
};

struct PWildPokemonHeader {
   uint8_t mapGroup, mapNum;
   uint32_t info[4];
};

struct PTrainerMonNoItemDefaultMoves {
   uint16_t iv;
   uint8_t lvl;
   uint16_t species;
};

struct PTrainerMonItemDefaultMoves {
   uint16_t iv;
   uint8_t lvl;
   uint16_t species, heldItem;
};

struct PTrainerMonNoItemCustomMoves {
   uint16_t iv;
   uint8_t lvl;
   uint16_t species, moves[MAX_MON_MOVES];
};

struct PTrainerMonItemCustomMoves {
   uint16_t iv;
   uint8_t lvl;
   uint16_t species, heldItem, moves[MAX_MON_MOVES];
};

struct PTrainer {
   uint8_t partyFlags, trainerClass, encounterMusic_gender, trainerPic;
   uint8_t trainerName[TRAINER_NAME_LENGTH + 1];
   uint16_t items[MAX_TRAINER_ITEMS];
   uint8_t doubleBattle;
   uint32_t aiFlags;
   uint8_t partySize;
   uint32_t party;
};

struct PStarters {
   uint16_t species[3];
};

struct PMappedRom {
   void *base;
   size_t size;
   struct PMapHeader *maps;
   const char **map_names;
   uint16_t map_count;
   struct PWildPokemonHeader *mons;
   uint16_t mon_count;
   struct PTrainer *trainers;
   uint16_t trainer_count;

   // NULL if not available
   struct PStarters *starters;
};

extern struct PMappedRom P_MAPPED_ROM;

void* pokemon_load(ptrdiff_t addr);
ptrdiff_t pokemon_deref(ptrdiff_t addr);

struct PMapHeader* pokemon_map_header_by_group_and_id(const uint8_t group, const uint8_t id);
uint16_t pokemon_map_index_from_group_and_id(const uint8_t group, const uint8_t id);

struct PMapEvents* pokemon_map_header_events(struct PMapHeader *header);
struct PWarpEvent* pokemon_map_events_warps(struct PMapEvents *events, uint8_t *out_num);

#define POKEMON_HEADER_EVENTS_WRAPPER(T, N) \
   static inline T pokemon_map_header_##N(struct PMapHeader *header, uint8_t *out_num) { \
      return pokemon_map_events_##N(pokemon_map_header_events(header), out_num); \
   }

POKEMON_HEADER_EVENTS_WRAPPER(struct PWarpEvent*, warps)

#undef POKEMON_HEADER_EVENTS_WRAPPER

void pokemon_map_rom(const char *path);
void pokemon_unmap_rom(void);
