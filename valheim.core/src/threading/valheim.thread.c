#include "valheim.thread.h"
#include "valheim.memory.h"

#include <Windows.h>

b8 valheim_createThread(valheim_ThreadCreateInfo *createInfo, valheim_Allocator *allocator, valheim_Thread *outThread) {
	void *params = createInfo->params;

	if (createInfo->ownsParams) {
		outThread->ownsParams = true;
		outThread->params = valheim_allocate(allocator, createInfo->paramsSize);
		valheim_copyMemory(outThread->params, createInfo->params, createInfo->paramsSize);
		params = outThread->params;
	}

	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)createInfo->entry, params, 0, NULL);
	if (!handle) {
		return false;
	}

	outThread->internalData = handle;
	return true;
}

void valheim_destroyThread(valheim_Thread *thread, valheim_Allocator *allocator) {
	if (thread && thread->internalData) {
		if (thread->params && thread->ownsParams) {
			valheim_free(allocator, thread->params);
		}

		CloseHandle(thread->internalData);
	}
}

b8 valheim_threadJoin(valheim_Thread *thread) {
	if (thread) {
		DWORD result = WaitForSingleObject(thread->internalData, INFINITE);
		return result == WAIT_OBJECT_0;
	}
	return false;
}

b8 valheim_threadDetach(valheim_Thread *thread) {
	if (thread) {
		CloseHandle(thread->internalData);
		return true;
	}
	return false;
}

void valheim_threadSleep(u64 milliseconds) {
	Sleep(milliseconds);
}
