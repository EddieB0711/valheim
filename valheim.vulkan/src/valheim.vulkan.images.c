//
// Created by Eddie Boyle on 9/10/2025.
//

#include "valheim.vulkan.images.h"
#include "valheim.vulkan.devices.h"

void valheim_transitionImageLayout(
	VkCommandBuffer commandBuffer,
	VkImage image,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkAccessFlags2 srcAccessMask,
	VkAccessFlags2 dstAccessMask,
	VkPipelineStageFlags2 srcStageMask,
	VkPipelineStageFlags dstStageMask,
	VkImageAspectFlags aspectMask
) {
	VkImageMemoryBarrier2 barrier = {0};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.image = image;
	barrier.srcStageMask = srcStageMask;
	barrier.dstStageMask = dstStageMask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange = (VkImageSubresourceRange){
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkDependencyInfo dependencyInfo = {0};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

VkFormat valheim_getSupportedFormats(valheim_VulkanContext *context, VkFormat *formats, u32 count, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (u32 iFormat = 0; iFormat < count; ++iFormat) {
		VkFormatProperties properties = {0};
		vkGetPhysicalDeviceFormatProperties(context->physicalDevice, formats[iFormat], &properties);

		if ((tiling == VK_IMAGE_TILING_LINEAR && ((properties.linearTilingFeatures & features) == features)) ||
				(tiling == VK_IMAGE_TILING_OPTIMAL && ((properties.linearTilingFeatures & features) == features))) {
			return formats[iFormat];
		}
	}

	return formats[0];
}

VkFormat valheim_getDepthFormat(valheim_VulkanContext *context) {
	VkFormat formats[] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	return valheim_getSupportedFormats(context, formats, VALHEIM_ARRAY_LEN(formats), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

b8 valheim_createImage(valheim_VulkanContext *context, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkSampleCountFlagBits samples, u32 width, u32 height, VkImage *outImage) {
	VkImageCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.extent = (VkExtent3D){.width = width, .height = height, .depth = 1};
	createInfo.usage = usage;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = samples;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	const VkResult result = vkCreateImage(context->device, &createInfo, NULL, outImage);
	return result == VK_SUCCESS;
}

b8 valheim_createImageView(valheim_VulkanContext *context, VkFormat format, VkImage image, VkImageAspectFlags aspect, VkImageView *outView) {
	VkImageViewCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange = (VkImageSubresourceRange){
		.aspectMask = aspect,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	const VkResult result = vkCreateImageView(context->device, &createInfo, NULL, outView);
	return result == VK_SUCCESS;
}

b8 valheim_allocateImageMemory(valheim_VulkanContext *context, VkImage image, VkDeviceMemory *outMemory) {
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(context->device, image, &requirements);

	VkMemoryAllocateInfo allocateInfo = {0};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = valheim_getMemoryTypeIndex(context, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkResult result = vkAllocateMemory(context->device, &allocateInfo, NULL, outMemory);
	if (result != VK_SUCCESS) {
		return false;
	}

	result = vkBindImageMemory(context->device, image, *outMemory, 0);
	if (result != VK_SUCCESS) {
		return false;
	}

	return true;
}

b8 valheim_createSampler(valheim_VulkanContext *context, VkSampler *outSampler) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(context->physicalDevice, &properties);

	VkSamplerCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	const VkResult result = vkCreateSampler(context->device, &createInfo, NULL, outSampler);
	return result == VK_SUCCESS;
}
