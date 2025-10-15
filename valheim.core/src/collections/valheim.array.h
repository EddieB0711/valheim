//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_ARRAY_H
#define VALHEIM_VALHEIM_ARRAY_H

#include "valheim.allocator.h"
#include "valheim.memory.h"

typedef struct valheim_Array {
	u8 *data;
	u64 stride;
	u64 length;
	u64 capacity;
} valheim_Array;

VALHEIM_API b8 valheim_initArray(u64 stride, u64 capacity, valheim_Allocator *allocator, valheim_Array *array);

VALHEIM_API void valheim_deinitArray(valheim_Array *array, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_arrayAppend(valheim_Array *array, const void *value, valheim_Allocator *allocator);

#define valheim_indexableArrayUnpack(arr) (u8**)&(arr).data, (arr).length, sizeof(*(arr).data), &(arr).capacity, (arr).allocator

#define valheim_IndexableArray(T) struct { T* data; u64 capacity; u64 length; valheim_Allocator* allocator; }

#define valheim_initIndexableArray(arr, cap, alloc) \
  ((arr).allocator = (alloc), \
  ((arr).capacity = (cap)), \
  ((arr).length = 0), \
  ((arr).data = valheim_allocate(alloc, sizeof(*(arr).data) * (cap))))

#define valheim_deinitIndexableArray(arr) \
  (valheim_free((arr).allocator, (arr).data), \
  (valheim_zeroMemory(&(arr), sizeof(arr))))

#define valheim_indexableArrayAppend(arr, val) \
  (!valheim_indexableArrayResize(valheim_indexableArrayUnpack(arr)) ? false : \
  ((arr).data[(arr).length++] = (val), true), true)

#define valheim_indexableArrayRemoveAt(arr, idx) valheim_indexableArrayRemoveItem(valheim_indexableArrayUnpack(arr), idx)

#define valheim_indexableArrayClear(arr) \
  (valheim_zeroMemory((arr).data, sizeof(*(arr).data) * (arr).capacity), \
  ((arr).length = 0)) 

VALHEIM_API b8 valheim_indexableArrayResize(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_indexableArrayRemoveItem(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator, u64 idx);

#endif //VALHEIM_VALHEIM_ARRAY_H