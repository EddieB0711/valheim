#pragma once

#include "valheim.defines.h"

typedef struct valheim_StackAllocationHeader {
	u8 padding;
} valheim_StackAllocationHeader;

typedef struct valheim_StackAllocation {
	u8 *pool;
	u64 poolSize;
	u64 offset;
} valheim_StackAllocation;

VALHEIM_API void valheim_initStackAllocator(u8 *pool, u64 poolSize, valheim_StackAllocation *stack);

VALHEIM_API void valheim_deinitStackAllocator(valheim_StackAllocation *stack);

VALHEIM_API void *valheim_stackAllocateAligned(valheim_StackAllocation *stack, u64 size, u64 alignment);

VALHEIM_API void *valheim_stackAllocate(valheim_StackAllocation *stack, u64 size);

VALHEIM_API void valheim_stackFree(valheim_StackAllocation *stack, void *data);

VALHEIM_API void valheim_stackReset(valheim_StackAllocation *stack);
