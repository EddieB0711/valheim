#pragma once

#include "valheim.vulkan.types.h"

VkCommandBuffer valheim_beginRendering(valheim_VulkanContext *context);

VkCommandBuffer valheim_endRendering(valheim_VulkanContext *context);

b8 valheim_beginScene(valheim_VulkanContext *context, valheim_Allocator *allocator);

b8 valheim_endScene(valheim_VulkanContext *context);