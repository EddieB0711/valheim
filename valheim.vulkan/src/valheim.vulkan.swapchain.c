//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.vulkan.swapchain.h"

#include <valheim.arena.allocator.h>

static void valheim_getSurfaceDetails(valheim_VulkanContext *context) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &context->capabilities);
}

static void valheim_getFormat(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	u32 count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &count, NULL);

	VkSurfaceFormatKHR *formats = valheim_allocate(allocator, count * sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &count, formats);

	for (u32 iFormat = 0; iFormat < count; ++iFormat) {
		if (formats[iFormat].format == VK_FORMAT_B8G8R8A8_SRGB && formats[iFormat].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			context->format = formats[iFormat];
			valheim_free(allocator, formats);
			return;
		}
	}

	context->format = formats[0];
	valheim_free(allocator, formats);
}

static void valheim_getPresentMode(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	u32 count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &count, NULL);

	VkPresentModeKHR *presentModes = valheim_allocate(allocator, count * sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &count, presentModes);

	for (u32 iPresentMode = 0; iPresentMode < count; ++iPresentMode) {
		if (presentModes[iPresentMode] == VK_PRESENT_MODE_MAILBOX_KHR) {
			context->presentMode = presentModes[iPresentMode];
			valheim_free(allocator, presentModes);
			return;
		}
	}

	context->presentMode = VK_PRESENT_MODE_FIFO_KHR;
	valheim_free(allocator, presentModes);
}

b8 valheim_initSwapChain(valheim_VulkanContext *context) {
	u8 memory[128] = {0};

	valheim_ArenaAllocator arena = {0};
	valheim_initArenaAllocator(memory, VALHEIM_ARRAY_LEN(memory), &arena);

	valheim_Allocator arenaAllocator = {0};
	valheim_initAllocatorFromArena(&arena, &arenaAllocator);

	valheim_ArenaRegion region = {0};
	valheim_beginArenaRegion(&arena, &region);
	valheim_getFormat(context, &arenaAllocator);
	valheim_endArenaRegion(&region);

	valheim_beginArenaRegion(&arena, &region);
	valheim_getPresentMode(context, &arenaAllocator);
	valheim_endArenaRegion(&region);

	valheim_getSurfaceDetails(context);

	context->imageCount = context->capabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context->surface;
	createInfo.minImageCount = context->imageCount;
	createInfo.presentMode = context->presentMode;
	createInfo.imageFormat = context->format.format;
	createInfo.imageColorSpace = context->format.colorSpace;
	createInfo.imageArrayLayers = 1;
	createInfo.imageExtent = context->capabilities.currentExtent;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = context->capabilities.currentTransform;
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkResult result = vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapChain);

	if (result != VK_SUCCESS) {
		return false;
	}

	vkGetSwapchainImagesKHR(context->device, context->swapChain, &context->imageCount, NULL);

	valheim_initArray(context->swapChainImages, context->imageCount, context->allocator);
	valheim_initArray(context->swapChainImageViews, context->imageCount, context->allocator);

	context->swapChainImages.length = context->imageCount;
	context->swapChainImageViews.length = context->imageCount;

	context->imageAvailableSemaphores.length = context->imageCount;
	context->renderFinishedSemaphores.length = context->imageCount;

	vkGetSwapchainImagesKHR(context->device, context->swapChain, &context->imageCount, context->swapChainImages.data);

	for (u32 iImage = 0; iImage < context->imageCount; ++iImage) {
		VkImageViewCreateInfo imageViewCreateInfo = {0};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = context->swapChainImages.data[iImage];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = context->format.format;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;

		result = vkCreateImageView(context->device, &imageViewCreateInfo, NULL, &context->swapChainImageViews.data[iImage]);
		if (result != VK_SUCCESS) {
			return false;
		}
	}

	return true;
}

void valheim_deinitSwapChain(valheim_VulkanContext *context) {
	if (context->swapChain) {
		vkDestroySwapchainKHR(context->device, context->swapChain, NULL);

		valheim_deinitArray(context->swapChainImages);
		valheim_deinitArray(context->swapChainImageViews);
	}
}

b8 valheim_recreateSwapChain(valheim_VulkanContext *context) {
	vkDeviceWaitIdle(context->device);
	valheim_getSurfaceDetails(context);
	valheim_deinitSwapChain(context);
	return valheim_initSwapChain(context);
}

