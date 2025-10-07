#include "valheim.arena.allocator.h"
#include "valheim.allocator.h"
#include "valheim.memory.h"

b8 valheim_initArenaAllocator(u8 *memory, u64 memorySize, valheim_ArenaAllocator *allocator) {
	allocator->memory = memory;
	allocator->memorySize = memorySize;
	allocator->currentOffset = 0;
	allocator->previousOffset = 0;
	return true;
}

void *valheim_arenaAllocateAligned(valheim_ArenaAllocator *allocator, u64 size, u64 align) {
	const u64 alignMask = align - 1;
	const u64 totalSize = size + alignMask & ~alignMask;

	if (allocator->currentOffset + totalSize > allocator->memorySize) {
		return NULL;
	}

	void *ptr = allocator->memory + allocator->currentOffset;
	allocator->previousOffset = allocator->currentOffset;
	allocator->currentOffset += totalSize;
	return ptr;
}

void *valheim_arenaAllocate(valheim_ArenaAllocator *allocator, u64 size) {
	return valheim_arenaAllocateAligned(allocator, size, VALHEIM_DEFAULT_ALIGN);
}

void valheim_resetArena(valheim_ArenaAllocator *allocator) {
	allocator->currentOffset = 0;
	allocator->previousOffset = 0;
}

void *valheim_arenaAllocatorAllocate(u64 size, u64 align, void *userData) {
	valheim_ArenaAllocator *allocator = userData;
	return valheim_arenaAllocateAligned(allocator, size, align);
}

void valheim_arenaAllocatorFree(void *ptr, void *userData) {}

void *valheim_arenaAllocatorReallocate(void *ptr, u64 size, void *userData) {
	valheim_ArenaAllocator *allocator = userData;
	void *newPtr = valheim_arenaAllocate(allocator, size);
	return newPtr;
}

b8 valheim_beginArenaRegion(valheim_ArenaAllocator *arena, valheim_ArenaRegion *region) {
	region->arena = arena;
	region->previousOffset = arena->previousOffset;
	region->currentOffset = arena->currentOffset;
	return true;
}

void valheim_endArenaRegion(valheim_ArenaRegion *region) {
	region->arena->previousOffset = region->previousOffset;
	region->arena->currentOffset = region->currentOffset;
}

b8 valheim_initAllocatorFromArena(valheim_ArenaAllocator *arena, valheim_Allocator *allocator) {
	allocator->allocate = valheim_arenaAllocatorAllocate;
	allocator->free = valheim_arenaAllocatorFree;
	allocator->reallocate = valheim_arenaAllocatorReallocate;
	allocator->userData = arena;
	return true;
}
