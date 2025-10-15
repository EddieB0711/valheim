#ifndef VALHEIM_VALHEIM_VULKAN_CAMERA_H
#define VALHEIM_VALHEIM_VULKAN_CAMERA_H

#include "valheim.vulkan.types.h"

void valheim_initCamera(valheim_VulkanContext *context);

void valheim_cameraMoveForward(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraMoveBackward(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraMoveLeft(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraMoveRight(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraMoveUp(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraMoveDown(valheim_VulkanContext *context, f32 deltaTime);

void valheim_cameraRotate(valheim_VulkanContext *context, f32 xoffset, f32 yoffset, f32 sensitivity);

void valheim_getCameraView(valheim_VulkanContext *context, mat4 view);

#endif //VALHEIM_VALHEIM_VULKAN_CAMERA_H