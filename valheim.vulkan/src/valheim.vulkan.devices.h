//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_DEVICES_H
#define VALHEIM_VALHEIM_VULKAN_DEVICES_H

#include "valheim.vulkan.types.h"

u32 valheim_getMemoryTypeIndex(valheim_VulkanContext *context, u32 typeFilter, VkMemoryPropertyFlags properties);

b8 valheim_selectPhysicalDevice(valheim_VulkanContext *context);

b8 valheim_initLogicalDevice(valheim_VulkanContext *context);

void valheim_deinitLogicalDevice(valheim_VulkanContext *context);

VkSampleCountFlagBits valheim_getMaxSampleCount(valheim_VulkanContext *context);

#endif //VALHEIM_VALHEIM_VULKAN_DEVICES_H