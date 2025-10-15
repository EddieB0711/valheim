#pragma once

#include "valheim.allocator.h"

typedef void(*valheim_ThreadStart)(void *);

typedef struct valheim_Thread {
	void *internalState;
	void *params;
	b8 ownsParams;
} valheim_Thread;

typedef enum valheim_ThreadInitFlags {
	valheim_ThreadInitFlags_None = 1 << 0,
	valheim_ThreadInitFlags_CreateSuspended = 1 << 1,
} valheim_ThreadInitFlags;

typedef struct valheim_ThreadInitInfo {
	valheim_ThreadStart entry;
	void *params;
	b8 ownsParams;
	u64 paramsSize;
	valheim_ThreadInitFlags flags;
} valheim_ThreadInitInfo;

VALHEIM_API b8 valheim_initThread(valheim_ThreadInitInfo *initInfo, valheim_Allocator *allocator, valheim_Thread *outThread);

VALHEIM_API void valheim_deinitThread(valheim_Thread *thread, valheim_Allocator *allocator);

VALHEIM_API b8 valheim_threadStart(valheim_Thread *thread);

VALHEIM_API b8 valheim_threadJoin(valheim_Thread *thread);

VALHEIM_API b8 valheim_threadDetach(valheim_Thread *thread);

VALHEIM_API void valheim_threadSleep(u64 milliseconds);

