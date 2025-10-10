#pragma once

#include "valheim.opengl.types.h"

#include <valheim.allocator.h>

VALHEIM_API b8 valheim_initContext( valheim_Allocator *allocator, valheim_Context *context );

VALHEIM_API void valheim_deinitContext( valheim_Context *context, valheim_Allocator *allocator );

VALHEIM_API void valheim_runApplication( valheim_Context *context, valheim_Allocator *allocator );