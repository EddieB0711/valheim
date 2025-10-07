//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_ARRAY_H
#define VALHEIM_VALHEIM_ARRAY_H

#include "valheim.allocator.h"
#include "valheim.memory.h"

#define valheim_arrayUnpack(arr) (u8**)&(arr).data, (arr).length, sizeof(*(arr).data), &(arr).capacity, (arr).allocator

#define valheim_Array(T) struct { T* data; u64 capacity; u64 length; valheim_Allocator* allocator; }

#define valheim_initArray(arr, cap, alloc) \
  ((arr).allocator = (alloc), \
  ((arr).capacity = (cap)), \
  ((arr).length = 0), \
  ((arr).data = valheim_allocate(alloc, sizeof(*(arr).data) * (cap))))

#define valheim_deinitArray(arr) \
  (valheim_free((arr).allocator, (arr).data), \
  (valheim_zeroMemory(&(arr), sizeof(arr))))

#define valheim_arrayAppend(arr, val) \
  (!valheim_resizeArray(valheim_arrayUnpack(arr)) ? false : \
  ((arr).data[(arr).length++] = (val), true), true)

#define valheim_arrayRemoveAt(arr, idx) valheim_removeArrayItem(valheim_arrayUnpack(arr), idx)

#define valheim_arrayClear(arr) \
  (valheim_zeroMemory((arr).data, sizeof(*(arr).data) * (arr).capacity), \
  ((arr).length = 0)) 

VALHEIM_API b8 valheim_resizeArray(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_removeArrayItem(u8 **items, u64 length, u64 stride, u64 *capacity, valheim_Allocator *allocator, u64 idx);

#endif //VALHEIM_VALHEIM_ARRAY_H