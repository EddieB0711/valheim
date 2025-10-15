#include "valheim.thread.h"
#include "valheim.memory.h"

#include <Windows.h>

b8 valheim_initThread(valheim_ThreadInitInfo *initInfo, valheim_Allocator *allocator, valheim_Thread *thread) {
	void *params = initInfo->params;

	if (initInfo->ownsParams) {
		thread->ownsParams = true;
		thread->params = valheim_allocate(allocator, initInfo->paramsSize);
		valheim_copyMemory(thread->params, initInfo->params, initInfo->paramsSize);
		params = thread->params;
	}

	DWORD createFlags = 0;

	if (initInfo->flags & valheim_ThreadInitFlags_CreateSuspended) {
		createFlags |= CREATE_SUSPENDED;
	}

	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)initInfo->entry, params, createFlags, NULL);
	if (!handle) {
		return false;
	}

	thread->internalState = handle;
	return true;
}

void valheim_deinitThread(valheim_Thread *thread, valheim_Allocator *allocator) {
	if (thread && thread->internalState) {
		if (thread->params && thread->ownsParams) {
			valheim_free(allocator, thread->params);
		}

		CloseHandle(thread->internalState);
	}
}

b8 valheim_threadStart(valheim_Thread *thread) {
	ResumeThread(thread->internalState);
	return true;
}

b8 valheim_threadJoin(valheim_Thread *thread) {
	if (thread) {
		DWORD result = WaitForSingleObject(thread->internalState, INFINITE);
		return result == WAIT_OBJECT_0;
	}
	return false;
}

b8 valheim_threadDetach(valheim_Thread *thread) {
	if (thread) {
		CloseHandle(thread->internalState);
		return true;
	}
	return false;
}

void valheim_threadSleep(u64 milliseconds) {
	Sleep(milliseconds);
}
