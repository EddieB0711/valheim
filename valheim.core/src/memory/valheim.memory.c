//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.memory.h"

#include <stdlib.h>
#include <string.h>

typedef struct valheim_MemoryHeader {
	u64 size;
	u64 alignMask;
} valheim_MemoryHeader;

valheim_Memory *valheim_getDefaultMemory(void) {
	static valheim_Memory memory = {0};
	return &memory;
}

void *valheim_memoryAllocate(valheim_Memory *memory, u64 size, u64 align) {
	const u64 alignMask = align - 1;
	const u64 totalSize = sizeof(valheim_MemoryHeader) + size + alignMask & ~alignMask;

	valheim_MemoryHeader *header = malloc(totalSize);

	if (!header) {
		return NULL;
	}

	valheim_zeroMemory(header, totalSize);

	header->size = totalSize;
	header->alignMask = alignMask;

	valheim_acquireSpinLock(&memory->spinLock);
	memory->totalAllocated += totalSize;
	memory->remainingAllocated += totalSize;
	valheim_releaseSpinLock(&memory->spinLock);

	return header + 1;
}

void valheim_memoryFree(valheim_Memory *memory, void *ptr) {
	valheim_MemoryHeader *header = (valheim_MemoryHeader *)ptr - 1;
	valheim_acquireSpinLock(&memory->spinLock);
	memory->remainingAllocated -= header->size;
	valheim_releaseSpinLock(&memory->spinLock);
	free(header);
}

void *valheim_memoryReallocate(valheim_Memory *memory, void *ptr, u64 size) {
	valheim_MemoryHeader *header = (valheim_MemoryHeader *)ptr - 1;

	const u64 alignMask = header->alignMask;
	const u64 totalSize = sizeof(valheim_MemoryHeader) + size + alignMask & ~alignMask;

	valheim_acquireSpinLock(&memory->spinLock);
	memory->totalAllocated -= header->size;
	memory->remainingAllocated -= header->size;
	valheim_releaseSpinLock(&memory->spinLock);

	valheim_MemoryHeader *newHeader = realloc(header, totalSize);

	if (!newHeader) {
		return NULL;
	}

	newHeader->size = totalSize;

	valheim_acquireSpinLock(&memory->spinLock);
	memory->totalAllocated += totalSize;
	memory->remainingAllocated += totalSize;
	valheim_releaseSpinLock(&memory->spinLock);

	return newHeader + 1;
}

void *valheim_zeroMemory(void *ptr, u64 size) {
	return memset(ptr, 0, size);
}

void *valheim_copyMemory(void *dst, const void *src, u64 size) {
	return memcpy(dst, src, size);
}
