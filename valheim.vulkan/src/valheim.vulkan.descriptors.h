//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_DESCRIPTORS_H
#define VALHEIM_VALHEIM_VULKAN_DESCRIPTORS_H

#include "valheim.vulkan.types.h"

typedef struct valheim_DescriptorBufferInfo {
	VkDescriptorBufferInfo *bufferInfo;
	VkDescriptorType type;
	u32 binding;
} valheim_DescriptorBufferInfo;

typedef struct valheim_DescriptorImageInfo {
	VkDescriptorImageInfo *imageInfo;
	VkDescriptorType type;
	u32 binding;
} valheim_DescriptorImageInfo;

typedef struct valheim_DescriptorSetCreateInfo {
	VkDescriptorType *descriptorTypes;
	u32 descriptorTypeCount;

	VkDescriptorSetLayout *destriptorSetLayouts;

	valheim_DescriptorBufferInfo *descriptorBufferInfo;
	u32 descriptorBufferInfoCount;

	valheim_DescriptorImageInfo *descriptorImageInfo;
	u32 descriptorImageInfoCount;
} valheim_DescriptorSetCreateInfo;

typedef struct valheim_DescriptorAddInfo {
	VkDescriptorPool descriptorPool;

	VkDescriptorSet *descriptorSets;
	u32 descriptorSetCount;
} valheim_DescriptorAddInfo;

void valheim_initDescriptorSetManager( valheim_VulkanContext *context, valheim_Allocator *allocator );

void valheim_deinitDescriptorSetManager( valheim_VulkanContext *context, valheim_Allocator *allocator );

void valheim_addDescriptorSet( valheim_VulkanContext *context, valheim_DescriptorAddInfo *addInfo, valheim_VulkanDescriptorSet *descriptorSet );

b8 valheim_initDescriptors( valheim_VulkanContext *context, valheim_DescriptorSetCreateInfo *createInfo, valheim_VulkanDescriptorSet *descriptorSet, valheim_Allocator *allocator );

#endif //VALHEIM_VALHEIM_VULKAN_DESCRIPTORS_H
