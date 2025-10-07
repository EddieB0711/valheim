#include "valheim.vulkan.sync.h"

b8 valheim_initSyncObjects(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	valheim_initArray(context->imageAvailableSemaphores, context->imageCount, context->allocator);
	valheim_initArray(context->renderFinishedSemaphores, context->imageCount, context->allocator);
	valheim_initArray(context->inFlightFences, context->imageCount, context->allocator);
	valheim_initArray(context->imagesInFlight, context->imageCount, context->allocator);

	context->inFlightFences.length = context->imageCount;
	context->imagesInFlight.length = context->imageCount;

	for (u32 iImage = 0; iImage < context->imageCount; ++iImage) {
		VkSemaphoreCreateInfo semaphoreCreateInfo = {0};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkResult result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &context->imageAvailableSemaphores.data[iImage]);
		if (result != VK_SUCCESS) {
			return false;
		}

		result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &context->renderFinishedSemaphores.data[iImage]);
		if (result != VK_SUCCESS) {
			return false;
		}

		VkFenceCreateInfo fenceCreateInfo = {0};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(context->device, &fenceCreateInfo, NULL, &context->inFlightFences.data[iImage]);
		if (result != VK_SUCCESS) {
			return false;
		}
	}

	return true;
}

void valheim_destroySyncObjects(valheim_VulkanContext *context, valheim_Allocator *allocator) {}
