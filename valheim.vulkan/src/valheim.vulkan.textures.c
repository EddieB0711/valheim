#include "valheim.vulkan.textures.h"
#include "valheim.vulkan.images.h"
#include "valheim.vulkan.buffers.h"
#include "valheim.vulkan.commands.h"
#include "valheim.vulkan.devices.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void valheim_initTextureManager(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	valheim_initIndexableArray(context->textureManager.images, VALHEIM_KIBIBYTE(1), allocator);
	valheim_initIndexableArray(context->textureManager.imageMemory, VALHEIM_KIBIBYTE(1), allocator);
	valheim_initIndexableArray(context->textureManager.imageViews, VALHEIM_KIBIBYTE(1), allocator);
	valheim_initIndexableArray(context->textureManager.imageSizes, VALHEIM_KIBIBYTE(1), allocator);
	valheim_initIndexableArray(context->textureManager.samplers, VALHEIM_KIBIBYTE(1), allocator);
	valheim_initIndexableArray(context->textureManager.freeList, VALHEIM_KIBIBYTE(1), allocator);

	context->textureManager.images.length = VALHEIM_KIBIBYTE(1);
	context->textureManager.imageMemory.length = VALHEIM_KIBIBYTE(1);
	context->textureManager.imageViews.length = VALHEIM_KIBIBYTE(1);
	context->textureManager.imageSizes.length = VALHEIM_KIBIBYTE(1);
	context->textureManager.samplers.length = VALHEIM_KIBIBYTE(1);
	context->textureManager.freeList.length = VALHEIM_KIBIBYTE(1);

	for (u32 i = 0; i < context->textureManager.images.length; i++) {
		context->textureManager.freeList.data[i] = true;
	}
}

void valheim_deinitTextureManager(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	for (u32 i = 0; i < context->textureManager.images.length; i++) {
		if (!context->textureManager.freeList.data[i]) {
			vkDestroyImage(context->device, context->textureManager.images.data[i], NULL);
			vkDestroyImageView(context->device, context->textureManager.imageViews.data[i], NULL);
			vkDestroySampler(context->device, context->textureManager.samplers.data[i], NULL);
			vkFreeMemory(context->device, context->textureManager.imageMemory.data[i], NULL);
		}
	}
}

valheim_VulkanTexture valheim_addTexture(valheim_VulkanContext *context, valheim_TextureAddInfo *addInfo) {
	for (u32 i = 0; i < context->textureManager.images.length; i++) {
		if (context->textureManager.freeList.data[i]) {
			context->textureManager.freeList.data[i] = false;
			context->textureManager.images.data[i] = addInfo->image;
			context->textureManager.imageViews.data[i] = addInfo->imageView;
			context->textureManager.imageMemory.data[i] = addInfo->memory;
			context->textureManager.imageSizes.data[i] = addInfo->size;
			context->textureManager.samplers.data[i] = addInfo->sampler;

			return (s32) i;
		}
	}

	return 0;
}

b8 valheim_initDepthTexture(valheim_VulkanContext *context) {
	VkFormat format = valheim_getDepthFormat(context);

	const u32 width = context->capabilities.currentExtent.width;
	const u32 height = context->capabilities.currentExtent.height;

	VkImage image;
	if (!valheim_createImage(context, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, valheim_getMaxSampleCount(context), width, height, &image)) {
		return false;
	}

	VkDeviceMemory memory;
	if (!valheim_allocateImageMemory(context, image, &memory)) {
		return false;
	}

	VkImageView view;
	if (!valheim_createImageView(context, format, image, VK_IMAGE_ASPECT_DEPTH_BIT, &view)) {
		return false;
	}

	VkDebugUtilsObjectNameInfoEXT nameInfo = {0};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectHandle = (u64) image;
	nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
	nameInfo.pObjectName = "Depth image";

	vkSetDebugUtilsObjectNameEXT(context->device, &nameInfo);

	nameInfo.objectHandle = (u64) memory;
	nameInfo.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
	nameInfo.pObjectName = "Depth memory";

	vkSetDebugUtilsObjectNameEXT(context->device, &nameInfo);

	valheim_TextureAddInfo addInfo = {0};
	addInfo.image = image;
	addInfo.memory = memory;
	addInfo.sampler = NULL;
	addInfo.imageView = view;
	addInfo.size = 0;

	context->depthTexture = valheim_addTexture(context, &addInfo);
	return true;
}

b8 valheim_initColorTexture(valheim_VulkanContext *context) {
	VkImage image;

	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	const u32 width = context->capabilities.currentExtent.width;
	const u32 height = context->capabilities.currentExtent.height;

	if (!valheim_createImage(context, context->format.format, usage, VK_IMAGE_TILING_OPTIMAL, valheim_getMaxSampleCount(context), width, height, &image)) {
		return false;
	}

	VkDeviceMemory memory;
	if (!valheim_allocateImageMemory(context, image, &memory)) {
		return false;
	}

	VkImageView view;
	if (!valheim_createImageView(context, context->format.format, image, VK_IMAGE_ASPECT_COLOR_BIT, &view)) {
		return false;
	}

	valheim_TextureAddInfo addInfo = {0};
	addInfo.image = image;
	addInfo.memory = memory;
	addInfo.sampler = NULL;
	addInfo.imageView = view;
	addInfo.size = 0;

	context->colorTexture = valheim_addTexture(context, &addInfo);
	return true;
}

b8 valheim_initTexture(valheim_VulkanContext *context, const char *filename, valheim_VulkanTexture *outTexture) {
	s32 width, height, channels;
	u8 *data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!data) {
		return false;
	}

	const VkDeviceSize size = width * height * 4;

	valheim_VulkanBuffer stagingBuffer;
	if (!valheim_initStagingBuffer(context, data, size, &stagingBuffer)) {
		stbi_image_free(data);
		return false;
	}

	VkImage image;
	if (!valheim_createImage(context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, width, height, &image)) {
		stbi_image_free(data);
		valheim_deinitBuffer(context, stagingBuffer);
		return false;
	}

	VkDeviceMemory memory;
	if (!valheim_allocateImageMemory(context, image, &memory)) {
		stbi_image_free(data);
		valheim_deinitBuffer(context, stagingBuffer);
		return false;
	}

	VkCommandBuffer commandBuffer;
	valheim_beginTransientCommand(context, &commandBuffer);

	valheim_transitionImageLayout(
		commandBuffer,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	valheim_BufferImageCopy copy = {0};
	copy.width = width;
	copy.height = height;
	copy.commandBuffer = commandBuffer;

	ValheimCopyBufferToImage(context, &copy, stagingBuffer, image);

	valheim_transitionImageLayout(
		commandBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	valheim_endTransientCommand(context, commandBuffer);
	valheim_deinitBuffer(context, stagingBuffer);

	VkImageView view;
	if (!valheim_createImageView(context, VK_FORMAT_R8G8B8A8_SRGB, image, VK_IMAGE_ASPECT_COLOR_BIT, &view)) {
		stbi_image_free(data);
		valheim_deinitBuffer(context, stagingBuffer);
		return false;
	}

	VkSampler sampler;
	if (!valheim_createSampler(context, &sampler)) {
		stbi_image_free(data);
		valheim_deinitBuffer(context, stagingBuffer);
		return false;
	}

	valheim_TextureAddInfo addInfo = {0};
	addInfo.image = image;
	addInfo.memory = memory;
	addInfo.sampler = sampler;
	addInfo.imageView = view;
	addInfo.size = size;

	*outTexture = valheim_addTexture(context, &addInfo);
	return true;
}

void valheim_destroyTexture(valheim_VulkanContext *context, valheim_VulkanTexture texture) {
	vkDestroyImage(context->device, context->textureManager.images.data[texture], NULL);
	vkFreeMemory(context->device, context->textureManager.imageMemory.data[texture], NULL);
	vkDestroySampler(context->device, context->textureManager.samplers.data[texture], NULL);
	vkDestroyImageView(context->device, context->textureManager.imageViews.data[texture], NULL);

	context->textureManager.freeList.data[texture] = true;
}
