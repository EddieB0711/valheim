//
// Created by Eddie Boyle on 9/10/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_BUFFERS_H
#define VALHEIM_VALHEIM_VULKAN_BUFFERS_H

#include "valheim.vulkan.types.h"

void valheim_initBufferManager(valheim_VulkanContext *context);

void valheim_deinitBufferManager(valheim_VulkanContext *context);

u32 valheim_addBuffer(valheim_VulkanContext *context, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size);

b8 valheim_createStagingBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

b8 valheim_createVertexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *Outbuffer);

b8 valheim_createIndexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

b8 valheim_initUniformBuffer(valheim_VulkanContext *context, VkDeviceSize size, valheim_VulkanBuffer *outBuffer);

void valheim_destroyBuffer(valheim_VulkanContext *context, valheim_VulkanBuffer buffer);

#endif //VALHEIM_VALHEIM_VULKAN_BUFFERS_H