#ifndef VALHEIM_VALHEIM_VULKAN_PIPELINE_H
#define VALHEIM_VALHEIM_VULKAN_PIPELINE_H

#include "valheim.vulkan.types.h"

typedef struct valheim_PipelineCreateInfo {
	const char *shaderFile;
} valheim_PipelineCreateInfo;

typedef struct valheim_PipelineAddInfo {
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkDescriptorSetLayout descriptorSetLayout;
} valheim_PipelineAddInfo;

void valheim_initPipelineManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

void valheim_deinitPipelineManager(valheim_VulkanContext *context, valheim_Allocator *allocator);

u32 valheim_addPipeline(valheim_VulkanContext *context, valheim_PipelineAddInfo *addInfo);

b8 valheim_initPipelineCache(valheim_VulkanContext *context);

b8 valheim_initPipeline(valheim_VulkanContext *context, valheim_PipelineCreateInfo *createInfo, valheim_VulkanPipeline *outPipeline);

b8 valheim_loadPipelines(valheim_VulkanContext *context, valheim_Allocator *allocator);

#endif //VALHEIM_VALHEIM_VULKAN_PIPELINE_H