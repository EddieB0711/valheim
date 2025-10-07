//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_SWAPCHAIN_H
#define VALHEIM_VALHEIM_VULKAN_SWAPCHAIN_H

#include "valheim.vulkan.types.h"

b8 valheim_initSwapChain(valheim_VulkanContext *context);

void valheim_deinitSwapChain(valheim_VulkanContext *context);

b8 valheim_recreateSwapChain(valheim_VulkanContext *context);

#endif //VALHEIM_VALHEIM_VULKAN_SWAPCHAIN_H