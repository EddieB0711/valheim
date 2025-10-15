#ifndef VALHEIM_VALHEIM_VULKAN_BUFFERS_H
#define VALHEIM_VALHEIM_VULKAN_BUFFERS_H

#include "valheim.vulkan.types.h"

void valheim_initBufferManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

void valheim_deinitBufferManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

u32 valheim_addBuffer(valheim_VulkanContext *context, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size);

b8 valheim_initStagingBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

b8 valheim_initVertexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *Outbuffer);

b8 valheim_initIndexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

b8 valheim_initUniformBuffer(valheim_VulkanContext *context, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

void valheim_deinitBuffer(valheim_VulkanContext *context, valheim_VulkanBuffer buffer);

#endif //VALHEIM_VALHEIM_VULKAN_BUFFERS_H