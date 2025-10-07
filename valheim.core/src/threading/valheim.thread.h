#pragma once

#include "valheim.allocator.h"

typedef void(*valheim_ThreadStart)(void *);

typedef struct valheim_Thread {
	void *internalData;
	void *params;
	b8 ownsParams;
} valheim_Thread;

typedef struct valheim_ThreadCreateInfo {
	valheim_ThreadStart entry;
	void *params;
	b8 ownsParams;
	u64 paramsSize;
} valheim_ThreadCreateInfo;

VALHEIM_API b8 valheim_createThread(valheim_ThreadCreateInfo *createInfo, valheim_Allocator *allocator, valheim_Thread *outThread);

VALHEIM_API void valheim_destroyThread(valheim_Thread *thread, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_threadJoin(valheim_Thread *thread);

VALHEIM_API b8 valheim_threadDetach(valheim_Thread *thread);

VALHEIM_API void valheim_threadSleep(u64 milliseconds);

