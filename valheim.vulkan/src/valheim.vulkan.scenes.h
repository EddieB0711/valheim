#ifndef VALHEIM_VALHEIM_VULKAN_SCENES_H
#define VALHEIM_VALHEIM_VULKAN_SCENES_H

#include "valheim.vulkan.types.h"

b8 valheim_initVulkanScene(valheim_VulkanContext *context, valheim_Allocator *allocator, valheim_VulkanScene *scene);

s32 valheim_vulkanSceneAddNode(valheim_VulkanScene *scene, s32 parent, s32 depth, valheim_Allocator *allocator);

void valheim_vulkanSceneRecalculateTransforms(valheim_VulkanScene *scene);

void valheim_vulkanSceneRender(valheim_VulkanContext *context, valheim_VulkanScene *scene, VkCommandBuffer commandBuffer, valheim_VulkanFrameData *frameData);

#endif //VALHEIM_VALHEIM_VULKAN_SCENES_H