#include <valheim.vulkan.backend.h>
#include <valheim.thread.h>

static void valheim_testThreadedAllocate(void *params) {
	valheim_Allocator *allocator = params;
	for (u32 i = 0; i < 100; ++i) {
		u8 *data = valheim_allocate(allocator, sizeof(u8) * 500);
		valheim_threadSleep(5);
		valheim_free(allocator, data);
	}
}

s32 main(void) {
	valheim_Allocator *heapAllocator = valheim_getHeapAllocator();

	valheim_Array(valheim_Thread) threads;
	valheim_initArray(threads, 10, heapAllocator);

	for (u32 i = 0; i < 10; ++i) {
		valheim_ThreadCreateInfo threadCreateInfo = {0};
		threadCreateInfo.entry = valheim_testThreadedAllocate;
		threadCreateInfo.ownsParams = false;
		threadCreateInfo.params = heapAllocator;

		valheim_Thread thread = {0};
		valheim_createThread(&threadCreateInfo, heapAllocator, &thread);
		valheim_arrayAppend(threads, thread);
	}

	for (u32 i = 0; i < 10; ++i) {
		valheim_threadJoin(&threads.data[i]);
		valheim_destroyThread(&threads.data[i], valheim_getHeapAllocator());
	}

	valheim_VulkanContext context;
	if (!valheim_initContext(heapAllocator, &context)) {
		return EXIT_FAILURE;
	}

	valheim_runApplication(&context);
	valheim_deinitContext(&context);
}
