//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_MAP_H
#define VALHEIM_VALHEIM_MAP_H

#include "valheim.allocator.h"
#include "valheim.memory.h"

typedef u64(*valheim_MapHash)(const u8 *, u64, u64);

typedef struct valheim_MapNode {
	void *data;
	u64 keyStride;
} valheim_MapNode;

typedef struct valheim_Map {
	valheim_MapNode **buckets;
	valheim_MapHash hash;
	u64 stride;
	u64 size;
	u64 capacity;
} valheim_Map;

VALHEIM_API b8 valheim_initMap(valheim_Map *map, u64 stride, u64 capacity, valheim_MapHash hash, valheim_Allocator *allocator);

VALHEIM_API void valheim_deinitMap(valheim_Map *map, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_mapInsert(valheim_Map *map, const void *key, u64 keySize, const void *value, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_mapFind(valheim_Map *map, const void *key, u64 keySize, void *value);

VALHEIM_API b8 valheim_mapUpdate(valheim_Map *map, const void *key, u64 keySize, const void *value);

//#define valheim_Map(k, V)                     \
//        struct {                              \
//                V* buckets;                   \
//                b8* bucketsInUse;             \
//                u64 capacity;                 \
//                valheim_Allocator* allocator; \
//                valheim_pfnMapHash hash;            \
//        }
//
//#define valheim_getMapHash(m, k, s) (m).hash((void*)(k), (s), 0) % (m).capacity
//
//#define valheim_initMap(m, cap, hashfn, alloc)                           \
//        (valheim_zeroMemory(&(m), sizeof(m)),                              \
//        ((m).capacity = (cap)),                                             \
//        ((m).hash = (hashfn)),                                              \
//        ((m).allocator = (alloc)),                                          \
//        ((m).buckets = valheim_allocate(alloc, sizeof(*(m).buckets) * (cap))), \
//        ((m).bucketsInUse = valheim_allocate(alloc, sizeof(b8) * (cap))))
//
//#define valheim_deinitMap(m)                             \
//        (valheim_free((m).allocator, (m).buckets),         \
//        (valheim_free((m).allocator, (m).bucketsInUse)), \
//        (valheim_zeroMemory(&(m), sizeof(m))))
//
//#define valheim_mapInsert(m, k, s, v)                 \
//        do {                                           \
//                u64 h = valheim_getMapHash(m, k, s); \
//                (m).buckets[h] = (v);                  \
//                (m).bucketsInUse[h] = true;          \
//        } while (0)
//
//#define valheim_mapFind(m, k, s, i)                             \
//        do {                                                     \
//                u64 h = valheim_getMapHash(m, k, s);           \
//                if ((m).bucketsInUse[h]) (i) = (m).buckets[h]; \
//        } while (0)

#endif //VALHEIM_VALHEIM_MAP_H
