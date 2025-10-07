#pragma once

#include "valheim.allocator.h"

typedef struct valheim_Mutex {
	void *internalState;
} valheim_Mutex;

VALHEIM_API b8 valheim_initMutex(valheim_Allocator *allocator, valheim_Mutex *mutex);

VALHEIM_API void valheim_deinitMutex(valheim_Mutex *mutex, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_mutexLock(valheim_Mutex *mutex);

VALHEIM_API b8 valheim_mutexUnlock(valheim_Mutex *mutex);