//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_ARENA_ALLOCATOR_H
#define VALHEIM_VALHEIM_ARENA_ALLOCATOR_H

#include "valheim.defines.h"

struct valheim_Allocator;

typedef struct valheim_ArenaAllocator {
	u8 *memory;
	u64 memorySize;
	u64 previousOffset;
	u64 currentOffset;
} valheim_ArenaAllocator;

typedef struct valheim_ArenaRegion {
	valheim_ArenaAllocator *arena;
	u64 previousOffset;
	u64 currentOffset;
} valheim_ArenaRegion;

VALHEIM_API b8 valheim_initArenaAllocator(u8 *memory, u64 memorySize, valheim_ArenaAllocator *allocator);

VALHEIM_API void *valheim_arenaAllocateAligned(valheim_ArenaAllocator *allocator, u64 size, u64 align);

VALHEIM_API void *valheim_arenaAllocate(valheim_ArenaAllocator *allocator, u64 size);

VALHEIM_API void valheim_resetArena(valheim_ArenaAllocator *allocator);

VALHEIM_API void *valheim_arenaAllocatorAllocate(u64 size, u64 align, void *userData);

VALHEIM_API void valheim_arenaAllocatorFree(void *ptr, void *userData);

VALHEIM_API void *valheim_arenaAllocatorReallocate(void *ptr, u64 size, void *userData);

VALHEIM_API b8 valheim_beginArenaRegion(valheim_ArenaAllocator *arena, valheim_ArenaRegion *region);

VALHEIM_API void valheim_endArenaRegion(valheim_ArenaRegion *region);

VALHEIM_API b8 valheim_initAllocatorFromArena(valheim_ArenaAllocator *arena, struct valheim_Allocator *allocator);

#endif //VALHEIM_VALHEIM_ARENA_ALLOCATOR_H
