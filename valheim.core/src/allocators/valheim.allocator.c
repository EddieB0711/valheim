//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.allocator.h"
#include "valheim.memory.h"

static void *valheim_defaultAllocate(u64 size, u64 align, void *userData) {
	return valheim_memoryAllocate(valheim_getDefaultMemory(), size, align);
}

static void valheim_defaultFree(void *ptr, void *userData) {
	valheim_memoryFree(valheim_getDefaultMemory(), ptr);
}

static void *valheim_defaultReallocate(void *ptr, u64 size, void *userData) {
	return valheim_memoryReallocate(valheim_getDefaultMemory(), ptr, size);
}

valheim_Allocator *valheim_getHeapAllocator(void) {
	static valheim_Allocator allocator = {
		.allocate = valheim_defaultAllocate,
		.free = valheim_defaultFree,
		.reallocate = valheim_defaultReallocate,
	};

	return &allocator;
}

void *valheim_allocate(valheim_Allocator *allocator, u64 size) {
	return allocator
		? allocator->allocate(size, VALHEIM_DEFAULT_ALIGN, allocator->userData)
		: valheim_getHeapAllocator()->allocate(size, VALHEIM_DEFAULT_ALIGN, NULL);
}

void *valheim_allocateAligned(valheim_Allocator *allocator, u64 size, u64 align) {
	return allocator
		? allocator->allocate(size, align, allocator->userData)
		: valheim_getHeapAllocator()->allocate(size, align, NULL);
}

void valheim_free(valheim_Allocator *allocator, void *ptr) {
	allocator
		? allocator->free(ptr, allocator->userData)
		: valheim_getHeapAllocator()->free(ptr, NULL);
}

void *valheim_reallocate(valheim_Allocator *allocator, void *ptr, u64 size) {
	return allocator
		? allocator->reallocate(ptr, size, allocator->userData)
		: valheim_getHeapAllocator()->reallocate(ptr, size, NULL);
}
