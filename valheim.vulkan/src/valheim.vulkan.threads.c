#include "valheim.vulkan.threads.h"

#include <Windows.h>

typedef struct valheim_ThreadPoolState {
	TP_CALLBACK_ENVIRON cbe;
	PTP_POOL pool;
} valheim_ThreadPoolState;

static VOID CALLBACK valheim_WorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work) {
	valheim_ThreadPoolWork *threadPoolWork = parameter;
	threadPoolWork->execute(threadPoolWork->params);
}

b8 valheim_initThreadPool(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	valheim_ThreadPoolState *state = valheim_allocate(allocator, sizeof *state);
	InitializeThreadpoolEnvironment(&state->cbe);

	state->pool = CreateThreadpool(NULL);

	if (state->pool == NULL) {
		return false;
	}

	SetThreadpoolCallbackPool(&state->cbe, state->pool);
	SetThreadpoolThreadMinimum(state->pool, 6);
	SetThreadpoolThreadMaximum(state->pool, 10);

	context->threadPool.internalState = state;
	return true;
}

b8 valheim_threadPoolSubmit(valheim_VulkanContext *context, void *params) {
	valheim_ThreadPoolState *state = context->threadPool.internalState;

	PTP_WORK work = CreateThreadpoolWork(valheim_WorkCallback, params, &state->cbe);

	if (work == NULL) {
		return false;
	}

	SubmitThreadpoolWork(work);
	return true;
}