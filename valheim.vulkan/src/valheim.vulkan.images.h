#ifndef VALHEIM_VALHEIM_VULKAN_IMAGES_H
#define VALHEIM_VALHEIM_VULKAN_IMAGES_H

#include "valheim.vulkan.types.h"

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
);

VkFormat valheim_getSupportedFormats( valheim_VulkanContext *context, VkFormat *formats, u32 count, VkImageTiling tiling, VkFormatFeatureFlags features );

VkFormat valheim_getDepthFormat( valheim_VulkanContext *context );

b8 valheim_createImage( valheim_VulkanContext *context, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkSampleCountFlagBits samples, u32 width, u32 height, VkImage *image );

b8 valheim_createImageView( valheim_VulkanContext *context, VkFormat format, VkImage image, VkImageAspectFlags aspect, VkImageView *view );

b8 valheim_allocateImageMemory( valheim_VulkanContext *context, VkImage image, VkDeviceMemory *memory );

b8 valheim_createSampler( valheim_VulkanContext *context, VkSampler *sampler );

#endif //VALHEIM_VALHEIM_VULKAN_IMAGES_H