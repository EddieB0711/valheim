#ifndef VALHEIM_VALHEIM_VULKAN_TEXTURES_H
#define VALHEIM_VALHEIM_VULKAN_TEXTURES_H

#include "valheim.vulkan.types.h"

typedef struct valheim_TextureAddInfo {
	VkImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkDeviceMemory memory;
	VkDeviceSize size;
} valheim_TextureAddInfo;

void valheim_initTextureManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

void valheim_deinitTextureManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

valheim_VulkanTexture valheim_addTexture(valheim_VulkanContext *context, valheim_TextureAddInfo *addInfo);

b8 valheim_initDepthTexture(valheim_VulkanContext *context);

b8 valheim_initColorTexture(valheim_VulkanContext *context);

b8 valheim_initTexture(valheim_VulkanContext *context, const char *filename, valheim_VulkanTexture *outTexture);

void valheim_destroyTexture(valheim_VulkanContext *context, valheim_VulkanTexture texture);

#endif //VALHEIM_VALHEIM_VULKAN_TEXTURES_H