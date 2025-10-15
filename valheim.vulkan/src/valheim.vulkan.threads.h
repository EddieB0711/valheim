#pragma once

#include "valheim.vulkan.types.h"

b8 valheim_initThreadPool(valheim_VulkanContext *context, valheim_Allocator *allocator);

b8 valheim_threadPoolSubmit(valheim_VulkanContext *context, void *params);
