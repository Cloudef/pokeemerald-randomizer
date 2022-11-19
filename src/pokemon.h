#pragma once

#include <stdbool.h>
#include <stdint.h>

static const uint16_t MAP_DYNAMIC = (0x7F | (0x7F << 8));
static const uint16_t MAP_UNDEFINED = (0xFF | (0xFF << 8));
static const uint8_t WARP_ID_SECRET_BASE = 0x7E;
static const uint8_t WARP_ID_DYNAMIC = 0x7F;
static const uint8_t WARP_ID_NONE = -1;

#define MAP_GROUP(map) (MAP_##map >> 8)
#define MAP_NUM(map) (MAP_##map & 0xFF)

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

struct PMappedRom {
   void *base;
   size_t size;
   struct PMapHeader *maps;
   const char **map_names;
   uint16_t map_count;
};

extern struct PMappedRom P_MAPPED_ROM;

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
