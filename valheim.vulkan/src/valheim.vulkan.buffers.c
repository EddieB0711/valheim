//
// Created by Eddie Boyle on 9/10/2025.
//

#include "valheim.vulkan.buffers.h"
#include "valheim.vulkan.devices.h"
#include "valheim.vulkan.commands.h"

#include <valheim.memory.h>

static b8 valheim_createBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, VkBufferUsageFlags usage, valheim_VulkanBuffer *outBuffer) {
	valheim_VulkanBuffer stagingHandle;
	if (!valheim_createStagingBuffer(context, data, size, &stagingHandle)) {
		return false;
	}

	VkBufferCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.usage = usage;
	createInfo.size = size;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	VkResult result = vkCreateBuffer(context->device, &createInfo, NULL, &buffer);
	if (result != VK_SUCCESS) {
		valheim_destroyBuffer(context, stagingHandle);
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(context->device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {0};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = valheim_getMemoryTypeIndex(context, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory memory;
	result = vkAllocateMemory(context->device, &allocateInfo, NULL, &memory);

	if (result != VK_SUCCESS) {
		valheim_destroyBuffer(context, stagingHandle);
		vkDestroyBuffer(context->device, buffer, NULL);
		return false;
	}

	vkBindBufferMemory(context->device, buffer, memory, 0);

	VkCommandBuffer commandBuffer;
	if (!valheim_beginTransientCommand(context, &commandBuffer)) {
		valheim_destroyBuffer(context, stagingHandle);
		vkDestroyBuffer(context->device, buffer, NULL);
		vkFreeMemory(context->device, memory, NULL);
		return false;
	}

	VkBufferCopy region = {0};
	region.size = size;

	vkCmdCopyBuffer(commandBuffer, context->bufferManager.buffers.data[stagingHandle], buffer, 1, &region);

	valheim_endTransientCommand(context, commandBuffer);
	valheim_destroyBuffer(context, stagingHandle);

	*outBuffer = (valheim_VulkanBuffer)valheim_addBuffer(context, buffer, memory, size);
	return true;
}

void valheim_initBufferManager(valheim_VulkanContext *context) {
	valheim_initArray(context->bufferManager.buffers, 1024, context->allocator);
	valheim_initArray(context->bufferManager.bufferMemory, 1024, context->allocator);
	valheim_initArray(context->bufferManager.bufferSizes, 1024, context->allocator);
	valheim_initArray(context->bufferManager.buffersInUse, 1024, context->allocator);

	context->bufferManager.buffers.length = 1024;
	context->bufferManager.bufferMemory.length = 1024;
	context->bufferManager.bufferSizes.length = 1024;
	context->bufferManager.buffersInUse.length = 1024;

	for (u32 iBuffer = 0; iBuffer < context->bufferManager.buffers.length; ++iBuffer) {
		context->bufferManager.buffersInUse.data[iBuffer] = false;
	}
}

void valheim_deinitBufferManager(valheim_VulkanContext *context) {
	for (u32 iBuffer = 0; iBuffer < 1024; ++iBuffer) {
		if (context->bufferManager.buffersInUse.data[iBuffer]) {
			vkDestroyBuffer(context->device, context->bufferManager.buffers.data[iBuffer], NULL);
			vkFreeMemory(context->device, context->bufferManager.bufferMemory.data[iBuffer], NULL);
		}
	}

	valheim_deinitArray(context->bufferManager.buffers);
	valheim_deinitArray(context->bufferManager.bufferMemory);
	valheim_deinitArray(context->bufferManager.buffersInUse);
	valheim_deinitArray(context->bufferManager.bufferSizes);
}

u32 valheim_addBuffer(valheim_VulkanContext *context, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size) {
	for (u32 iBuffer = 0; iBuffer < context->bufferManager.buffers.length; iBuffer++) {
		if (context->bufferManager.buffersInUse.data[iBuffer] == false) {
			context->bufferManager.buffers.data[iBuffer] = buffer;
			context->bufferManager.bufferMemory.data[iBuffer] = memory;
			context->bufferManager.bufferSizes.data[iBuffer] = size;
			context->bufferManager.buffersInUse.data[iBuffer] = true;
			return iBuffer;
		}
	}

	return 0xFFFFFFF;
}

b8 valheim_createStagingBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer) {
	VkBufferCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.size = size;

	VkBuffer stagingBuffer;
	VkResult result = vkCreateBuffer(context->device, &createInfo, NULL, &stagingBuffer);

	if (result != VK_SUCCESS) {
		return false;
	}

	VkMemoryRequirements memoryRequirements = {0};
	vkGetBufferMemoryRequirements(context->device, stagingBuffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {0};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = valheim_getMemoryTypeIndex(context, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDeviceMemory memory = VK_NULL_HANDLE;
	vkAllocateMemory(context->device, &allocateInfo, NULL, &memory);
	vkBindBufferMemory(context->device, stagingBuffer, memory, 0);

	void *deviceData;
	vkMapMemory(context->device, memory, 0, size, 0, &deviceData);
	valheim_copyMemory(deviceData, data, size);
	vkUnmapMemory(context->device, memory);

	*outBuffer = (s32)valheim_addBuffer(context, stagingBuffer, memory, size);
	return true;
}

b8 valheim_createVertexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer) {
	return valheim_createBuffer(context, data, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, outBuffer);
}

b8 valheim_createIndexBuffer(valheim_VulkanContext *context, const void *data, VkDeviceSize size, valheim_VulkanBuffer *outBuffer) {
	return valheim_createBuffer(context, data, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, outBuffer);
}

b8 valheim_initUniformBuffer(valheim_VulkanContext *context, VkDeviceSize size, valheim_VulkanBuffer *outBuffer) {
	VkBufferCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	createInfo.size = size;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	VkResult result = vkCreateBuffer(context->device, &createInfo, NULL, &buffer);

	if (result != VK_SUCCESS) {
		return false;
	}

	VkMemoryRequirements memoryRequirements = {0};
	vkGetBufferMemoryRequirements(context->device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {0};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = valheim_getMemoryTypeIndex(context, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDeviceMemory memory;
	result = vkAllocateMemory(context->device, &allocateInfo, NULL, &memory);

	if (result != VK_SUCCESS) {
		vkDestroyBuffer(context->device, buffer, NULL);
		return false;
	}

	vkBindBufferMemory(context->device, buffer, memory, 0);

	*outBuffer = (s32)valheim_addBuffer(context, buffer, memory, size);
	return true;
}

void valheim_destroyBuffer(valheim_VulkanContext *context, valheim_VulkanBuffer buffer) {
	vkDestroyBuffer(context->device, context->bufferManager.buffers.data[buffer], NULL);
	vkFreeMemory(context->device, context->bufferManager.bufferMemory.data[buffer], NULL);

	context->bufferManager.buffers.data[buffer] = VK_NULL_HANDLE;
	context->bufferManager.bufferMemory.data[buffer] = VK_NULL_HANDLE;
	context->bufferManager.bufferSizes.data[buffer] = 0;
	context->bufferManager.buffersInUse.data[buffer] = false;
}