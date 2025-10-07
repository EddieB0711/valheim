#include "valheim.stack.allocator.h"
#include "valheim.memory.h"

static u64 valheim_getAddressPadding(u64 block, u64 alignment, u64 stride) {
	u64 alignmentMask = alignment - 1;
	u64 modulo = block & alignmentMask;
	u64 padding = 0;

	if (modulo != 0) {
		padding = alignment - modulo;
	}

	u64 spaceNeeded = stride;
	if (padding < spaceNeeded) {
		spaceNeeded -= padding;
		if ((spaceNeeded & alignmentMask) != 0) {
			padding += alignment * (1 + (spaceNeeded / alignment));
		} else {
			padding += alignment * (spaceNeeded / alignment);
		}
	}

	return padding;
}

void valheim_initStackAllocator(u8 *pool, u64 poolSize, valheim_StackAllocation *stack) {
	stack->pool = pool;
	stack->poolSize = poolSize;
	stack->offset = 0;
}

void valheim_deinitStackAllocator(valheim_StackAllocation *stack) {
	stack->offset = 0;
}

void *valheim_stackAllocateAligned(valheim_StackAllocation *stack, u64 size, u64 alignment) {
	u8 *block = stack->pool + stack->offset;
	u64 padding = valheim_getAddressPadding((u64)block, alignment, sizeof(valheim_StackAllocationHeader));

	if (stack->offset + padding + size > stack->poolSize) {
		return NULL;
	}

	stack->offset += padding;

	block += padding;
	valheim_StackAllocationHeader *header = block - sizeof(*header);
	header->padding = padding;

	stack->offset += size;
	return valheim_zeroMemory(header + 1, size);
}

void *valheim_stackAllocate(valheim_StackAllocation *stack, u64 size) {
	return valheim_stackAllocateAligned(stack, size, VALHEIM_DEFAULT_ALIGN);
}

void valheim_stackFree(valheim_StackAllocation *stack, void *data) {
	if (data) {
		valheim_StackAllocationHeader *header = (valheim_StackAllocationHeader *)data - 1;

		u64 CurrentAddress = (u64)data;
		u64 PreviousOffset = CurrentAddress - header->padding - (u64)stack->pool;

		stack->offset = PreviousOffset;
	}
}

void valheim_stackReset(valheim_StackAllocation *stack) {
	stack->offset = 0;
}