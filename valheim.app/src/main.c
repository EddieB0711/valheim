#include <valheim.vulkan.backend.h>

s32 main( s32 argc, char **argv ) {
	valheim_Allocator *heapAllocator = valheim_getHeapAllocator();

	valheim_VulkanContext context;
	if ( !valheim_initContext( heapAllocator, &context ) ) {
		return EXIT_FAILURE;
	}

	valheim_runApplication( &context, heapAllocator );
	valheim_deinitContext( &context, heapAllocator );
}
