//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_ALLOCATOR_H
#define VALHEIM_VALHEIM_ALLOCATOR_H

#include "valheim.defines.h"

typedef struct valheim_Allocator {
	void *(*allocate)(u64, u64, void *);
	void (*free)(void *, void *);
	void *(*reallocate)(void *, u64, void *);
	void *userData;
} valheim_Allocator;

VALHEIM_API valheim_Allocator *valheim_getHeapAllocator(void);

VALHEIM_API void *valheim_allocate(valheim_Allocator *allocator, u64 size);

VALHEIM_API void *valheim_allocateAligned(valheim_Allocator *allocator, u64 size, u64 align);

VALHEIM_API void valheim_free(valheim_Allocator *allocator, void *ptr);

VALHEIM_API void *valheim_reallocate(valheim_Allocator *allocator, void *ptr, u64 size);

#endif //VALHEIM_VALHEIM_ALLOCATOR_H
