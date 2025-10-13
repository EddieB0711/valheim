//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.vulkan.commands.h"

b8 valheim_initCommandPool( valheim_VulkanContext *context ) {
	VkCommandPoolCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = context->graphicsQueueIndex;

	VkResult result = vkCreateCommandPool( context->device, &createInfo, NULL, &context->commandPool );
	return result == VK_SUCCESS;
}

void valheim_deinitCommandPool( valheim_VulkanContext *context ) {
	if ( context->commandPool ) {
		vkDestroyCommandPool( context->device, context->commandPool, NULL );
	}
}

b8 valheim_initCommandBuffers( valheim_VulkanContext *context, valheim_Allocator *allocator ) {
	VkCommandBufferAllocateInfo allocateInfo = { 0 };
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = context->commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = context->imageCount;

	valheim_initIndexableArray( context->commandBuffers, context->imageCount, allocator );
	context->commandBuffers.length = context->imageCount;

	const VkResult result = vkAllocateCommandBuffers( context->device, &allocateInfo, context->commandBuffers.data );
	return result == VK_SUCCESS;
}

void valheim_deinitCommandBuffers( valheim_VulkanContext *context, valheim_Allocator *allocator ) {
	vkFreeCommandBuffers( context->device, context->commandPool, context->imageCount, context->commandBuffers.data );
}

b8 valheim_beginTransientCommand( valheim_VulkanContext *context, VkCommandBuffer *commandBuffer ) {
	VkCommandBufferAllocateInfo allocateInfo = { 0 };
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = context->commandPool;
	allocateInfo.commandBufferCount = 1;

	VkResult result = vkAllocateCommandBuffers( context->device, &allocateInfo, commandBuffer );
	if ( result != VK_SUCCESS ) {
		return false;
	}

	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer( *commandBuffer, &beginInfo );
	return result == VK_SUCCESS;
}

b8 valheim_endTransientCommand( valheim_VulkanContext *context, VkCommandBuffer commandBuffer ) {
	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkResult result = vkQueueSubmit( context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	if ( result != VK_SUCCESS ) {
		return false;
	}

	result = vkQueueWaitIdle( context->graphicsQueue );
	return result == VK_SUCCESS;
}

VkCommandBuffer valheim_beginCommandBuffer( valheim_VulkanContext *context ) {
	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer( context->commandBuffers.data[ context->currentFrame ], &beginInfo );
	return context->commandBuffers.data[ context->currentFrame ];
}

void valheim_endCommandBuffer( valheim_VulkanContext *context, VkCommandBuffer commandBuffer ) {
	vkEndCommandBuffer( commandBuffer );
}

void ValheimCopyBufferToImage( valheim_VulkanContext *context, valheim_BufferImageCopy *imageCopy, valheim_VulkanBuffer buffer, VkImage image ) {
	VkBufferImageCopy region = { 0 };
	region.imageExtent = ( VkExtent3D ){ imageCopy->width, imageCopy->height, 1 };
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;

	vkCmdCopyBufferToImage( imageCopy->commandBuffer, context->bufferManager.buffers.data[ buffer ], image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
}
