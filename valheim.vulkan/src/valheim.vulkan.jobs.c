#include "valheim.vulkan.jobs.h"

#include <Windows.h>

typedef struct valheim_EngineInitInfo {
	valheim_VulkanContext *context;
	valheim_VulkanInitializationGraph *graph;
	valheim_VulkanInitializationGraphNode *node;
	valheim_Allocator *allocator;
	LPVOID mainFiber;
} valheim_EngineInitInfo;

static b8 valheim_initEngineRoutine( valheim_EngineInitInfo *initInfo ) {
	
}

b8 valheim_initEngineJob( valheim_VulkanContext *context, valheim_VulkanInitializationGraph *graph, valheim_Allocator *allocator ) {
	LPVOID mainFiber = ConvertThreadToFiber( NULL );
	ConvertFiberToThread();
}
