#include "valheim.vulkan.jobs.h"
#include "valheim.vulkan.threads.h"

#include <valheim.hashing.h>
#include <valheim.strings.h>

#include <Windows.h>

typedef struct valheim_EngineInitInfo {
	valheim_VulkanContext *context;
	valheim_VulkanInitializationGraph *graph;
	valheim_VulkanInitializationGraphNode *node;
	valheim_Allocator *allocator;
	LPVOID mainFiber;
} valheim_EngineInitInfo;

typedef struct valheim_EngineInitJob {
	valheim_Map fibers;
	valheim_IndexableArray(valheim_EngineInitInfo) initInfo;
} valheim_EngineInitJob;

static b8 valheim_initEngineRoutine(valheim_EngineInitInfo *initInfo) {
	
}

b8 valheim_initEngineJob(valheim_VulkanContext *context, valheim_VulkanInitializationGraph *graph, valheim_Allocator *allocator) {
	LPVOID mainFiber = ConvertThreadToFiber(NULL);
	ConvertFiberToThread();
}
