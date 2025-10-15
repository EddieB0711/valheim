#pragma once

#include "valheim.vulkan.types.h"

b8 valheim_initSyncObjects(valheim_VulkanContext *context, valheim_Allocator *allocator);

void valheim_destroySyncObjects(valheim_VulkanContext *context, valheim_Allocator *allocator);