//
// Created by Eddie Boyle on 9/11/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_GLTF_H
#define VALHEIM_VALHEIM_VULKAN_GLTF_H

#include "valheim.vulkan.types.h"

b8 valheim_loadGltfFile( valheim_VulkanContext *context, const char *file, valheim_Allocator *allocator, valheim_VulkanScene **outScene );

#endif //VALHEIM_VALHEIM_VULKAN_GLTF_H