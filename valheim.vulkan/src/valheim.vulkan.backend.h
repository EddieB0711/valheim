#ifndef VALHEIM_VALHEIM_VULKAN_BACKEND_H
#define VALHEIM_VALHEIM_VULKAN_BACKEND_H

#include "valheim.vulkan.types.h"

VALHEIM_API b8 valheim_initContext(valheim_Allocator *allocator, valheim_VulkanContext *context);

VALHEIM_API void valheim_deinitContext(valheim_VulkanContext *context, valheim_Allocator *allocator);

VALHEIM_API void valheim_runApplication(valheim_VulkanContext *context, valheim_Allocator *allocator);

#endif //VALHEIM_VALHEIM_VULKAN_BACKEND_H