#include <valheim.opengl.backend.h>

s32 main( s32 argc, char **argv ) {
	valheim_Allocator *allocator = valheim_getHeapAllocator();

	valheim_Context context;
	valheim_initContext( allocator, &context );

	valheim_runApplication( &context, allocator );
	valheim_deinitContext( &context, allocator );
}