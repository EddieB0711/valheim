#pragma once

#include "valheim.defines.h"

typedef void(*valheim_FiberEntry)(void *);

typedef enum valheim_FiberState {
	valheim_FiberStateRunning,
	valheim_FiberStatePaused,
} valheim_FiberState;

typedef struct valheim_FiberContext {
	u64 RIP;
	u64 RSP;
	u64 RBX;
	u64 RBP;
	u64 R12;
	u64 R13;
	u64 R14;
	u64 R15;
	u64 MXCSR;
	u64 X86FCW;
} valheim_FiberContext;

typedef struct valheim_Fiber {
	void *stack;
	void *args;

	valheim_FiberContext context;
	valheim_FiberState state;
	valheim_FiberEntry entry;
} valheim_Fiber;

VALHEIM_API b8 valheim_createFiber(void *stack, u64 stackSize, valheim_FiberEntry entry, void *args, valheim_Fiber *outFiber);

//extern "C" void valheim_switchContext(valheim_FiberContext* OldContext, valheim_FiberContext* NewContext);