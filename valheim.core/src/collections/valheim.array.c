//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.array.h"

b8 valheim_resizeArray(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator) {
	if (length >= *capacity) {
		const u64 newCapacity = *capacity == 0 ? 1 : *capacity << 1;

		void *memory = valheim_reallocate(allocator, *items, stride * newCapacity);
		if (memory == NULL) {
			return false;
		}

		*items = memory;
		*capacity = newCapacity;
	}

	return true;
}

b8
valheim_removeArrayItem(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator, u64 idx) {
	if (idx >= length) {
		return false;
	}

	if (idx == length - 1) {
		valheim_zeroMemory((*items) + stride * idx, stride);
	} else {
		valheim_copyMemory((*items) + stride * idx, (*items) + stride * (idx + 1), stride * (length - idx));
	}

	return true;
}
