//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_COMMANDS_H
#define VALHEIM_VALHEIM_VULKAN_COMMANDS_H

#include "valheim.vulkan.types.h"

typedef struct valheim_BufferImageCopy {
	VkCommandBuffer commandBuffer;
	u32 width;
	u32 height;
} valheim_BufferImageCopy;

b8 valheim_initCommandPool(valheim_VulkanContext *context);

void valheim_deinitCommandPool(valheim_VulkanContext *context);

b8 valheim_acquireCommandBuffers(valheim_VulkanContext *context);

void valheim_releaseCommandBuffers(valheim_VulkanContext *context);

b8 valheim_beginTransientCommand(valheim_VulkanContext *context, VkCommandBuffer *commandBuffer);

b8 valheim_endTransientCommand(valheim_VulkanContext *context, VkCommandBuffer commandBuffer);

VkCommandBuffer valheim_beginCommandBuffer(valheim_VulkanContext *context);

void valheim_endCommandBuffer(valheim_VulkanContext *context, VkCommandBuffer commandBuffer);

void ValheimCopyBufferToImage(valheim_VulkanContext *context, valheim_BufferImageCopy *imageCopy, valheim_VulkanBuffer buffer, VkImage image);

#endif //VALHEIM_VALHEIM_VULKAN_COMMANDS_H