//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_MEMORY_H
#define VALHEIM_VALHEIM_MEMORY_H

#include "valheim.defines.h"
#include "valheim.spin.lock.h"

typedef struct valheim_Memory {
	u64 totalAllocated;
	u64 remainingAllocated;
	valheim_SpinLock spinLock;
} valheim_Memory;

VALHEIM_API valheim_Memory *valheim_getDefaultMemory(void);

VALHEIM_API void *valheim_memoryAllocate(valheim_Memory *memory, u64 size, u64 align);

VALHEIM_API void valheim_memoryFree(valheim_Memory *memory, void *ptr);

VALHEIM_API void *valheim_memoryReallocate(valheim_Memory *memory, void *ptr, u64 size);

VALHEIM_API void *valheim_zeroMemory(void *ptr, u64 size);

VALHEIM_API void *valheim_copyMemory(void *dst, const void *src, u64 size);

#endif //VALHEIM_VALHEIM_MEMORY_H
