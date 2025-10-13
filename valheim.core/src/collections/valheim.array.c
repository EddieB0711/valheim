#include "valheim.array.h"

b8 valheim_initArray( u64 stride, u64 capacity, valheim_Allocator *allocator, valheim_Array *array ) {
	array->stride = stride;
	array->capacity = capacity;
	array->length = 0;
	array->data = valheim_allocate( allocator, stride * capacity );
	return array->data != NULL;
}

void valheim_deinitArray( valheim_Array *array, valheim_Allocator *allocator ) {
	array->length = 0;
	array->stride = 0;
	array->capacity = 0;
	valheim_free( allocator, array->data );
	array->data = NULL;
}

b8 valheim_arrayAppend( valheim_Array *array, const void *value, valheim_Allocator *allocator ) {
	if ( array->length == array->capacity ) {
		array->capacity *= 2;
		array->data = valheim_reallocate( allocator, array->data, array->capacity * array->stride );
		if ( !array->data ) {
			return false;
		}
	}

	valheim_copyMemory( array->data + array->length++ * array->stride, value, array->stride );
	return true;
}

b8 valheim_indexableArrayResize(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator) {
	if (length >= *capacity) {
		const u64 newCapacity = *capacity <= 1 ? 2 : *capacity << 1;

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
valheim_indexableArrayRemoveItem(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator, u64 idx) {
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
