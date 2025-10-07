//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.vulkan.descriptors.h"

#include <valheim.arena.allocator.h>

void valheim_initDescriptorSetManager(valheim_VulkanContext *context) {
	valheim_initArray(context->descriptorSetManager.descriptorPools, 1024, context->allocator);
	valheim_initArray(context->descriptorSetManager.descriptorSets, 1024, context->allocator);
	valheim_initArray(context->descriptorSetManager.freeList, 1024, context->allocator);
	valheim_initArray(context->descriptorSetManager.descriptorSetCounts, 1024, context->allocator);

	context->descriptorSetManager.descriptorPools.length = 1024;
	context->descriptorSetManager.descriptorSets.length = 1024;
	context->descriptorSetManager.freeList.length = 1024;
	context->descriptorSetManager.descriptorSetCounts.length = 1024;

	for (u32 iSet = 0; iSet < context->descriptorSetManager.descriptorSets.length; ++iSet) {
		context->descriptorSetManager.freeList.data[iSet] = true;
	}
}

void valheim_deinitDescriptorSetManager(valheim_VulkanContext *context) {
	for (u32 iSet = 0; iSet < context->descriptorSetManager.freeList.length; ++iSet) {
		if (!context->descriptorSetManager.freeList.data[iSet]) {
			vkDestroyDescriptorPool(context->device, context->descriptorSetManager.descriptorPools.data[iSet], NULL);
			valheim_free(context->allocator, context->descriptorSetManager.descriptorSets.data[iSet]);
		}
	}

	valheim_deinitArray(context->descriptorSetManager.descriptorPools);
	valheim_deinitArray(context->descriptorSetManager.descriptorSets);
	valheim_deinitArray(context->descriptorSetManager.descriptorSetCounts);
	valheim_deinitArray(context->descriptorSetManager.freeList);
}

void valheim_addDescriptorSet(valheim_VulkanContext *context, valheim_DescriptorAddInfo *addInfo, valheim_VulkanDescriptorSet *outDescriptorSet) {
	*outDescriptorSet = -1;

	for (u32 iSet = 0; iSet < context->descriptorSetManager.descriptorSets.length; ++iSet) {
		if (context->descriptorSetManager.freeList.data[iSet]) {
			context->descriptorSetManager.freeList.data[iSet] = false;
			context->descriptorSetManager.descriptorSets.data[iSet] = addInfo->descriptorSets;
			context->descriptorSetManager.descriptorPools.data[iSet] = addInfo->descriptorPool;
			context->descriptorSetManager.descriptorSetCounts.data[iSet] = addInfo->descriptorSetCount;
			*outDescriptorSet = (s32)iSet;
			break;
		}
	}
}

b8 valheim_createDescriptors(valheim_VulkanContext *context, valheim_DescriptorSetCreateInfo *createInfo, valheim_VulkanDescriptorSet *outDescriptorSet) {
	VkDescriptorPoolSize descriptorPoolSizes[10] = {0};
	for (u32 iType = 0; iType < createInfo->descriptorTypeCount; ++iType) {
		descriptorPoolSizes[iType].type = createInfo->descriptorTypes[iType];
		descriptorPoolSizes[iType].descriptorCount = context->imageCount;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInf = {0};
	descriptorPoolCreateInf.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInf.maxSets = context->imageCount;
	descriptorPoolCreateInf.poolSizeCount = createInfo->descriptorTypeCount;
	descriptorPoolCreateInf.pPoolSizes = descriptorPoolSizes;

	VkDescriptorPool descriptorPool;
	VkResult result = vkCreateDescriptorPool(context->device, &descriptorPoolCreateInf, NULL, &descriptorPool);

	if (result != VK_SUCCESS) {
		return false;
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {0};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = context->imageCount;
	descriptorSetAllocateInfo.pSetLayouts = createInfo->destriptorSetLayouts;

	VkDescriptorSet *descriptorSets = valheim_allocate(context->allocator, sizeof(VkDescriptorSet) * context->imageCount);
	result = vkAllocateDescriptorSets(context->device, &descriptorSetAllocateInfo, descriptorSets);

	if (result != VK_SUCCESS) {
		vkDestroyDescriptorPool(context->device, descriptorPool, NULL);
		return false;
	}

	u8 memory[VALHEIM_KIBIBYTE(5)] = {0};

	valheim_ArenaAllocator arena = {0};
	valheim_initArenaAllocator(memory, VALHEIM_ARRAY_LEN(memory), &arena);

	for (u32 iImage = 0; iImage < context->imageCount; ++iImage) {
		u32 writeCount = createInfo->descriptorBufferInfoCount + createInfo->descriptorImageInfoCount;
		u32 writeIndex = 0;

		VkWriteDescriptorSet *writeSets = valheim_arenaAllocate(&arena, sizeof(VkWriteDescriptorSet) * writeCount);

		for (u32 iBuffer = 0; iBuffer < createInfo->descriptorBufferInfoCount; ++iBuffer) {
			writeSets[writeIndex].dstSet = descriptorSets[iImage];
			writeSets[writeIndex].dstBinding = createInfo->descriptorBufferInfo[iBuffer].binding;
			writeSets[writeIndex].descriptorCount = 1;
			writeSets[writeIndex].descriptorType = createInfo->descriptorBufferInfo[iBuffer].type;
			writeSets[writeIndex++].pBufferInfo = createInfo->descriptorBufferInfo[iBuffer].bufferInfo;
		}

		for (u32 iImageInfo = 0; iImageInfo < createInfo->descriptorImageInfoCount; ++iImageInfo) {
			writeSets[writeIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeSets[writeIndex].dstSet = descriptorSets[iImage];
			writeSets[writeIndex].dstBinding = createInfo->descriptorImageInfo[iImageInfo].binding;
			writeSets[writeIndex].descriptorCount = 1;
			writeSets[writeIndex].descriptorType = createInfo->descriptorImageInfo[iImageInfo].type;
			writeSets[writeIndex++].pImageInfo = createInfo->descriptorImageInfo[iImageInfo].imageInfo;
		}

		vkUpdateDescriptorSets(context->device, writeCount, writeSets, 0, NULL);
		valheim_resetArena(&arena);
	}

	valheim_DescriptorAddInfo addInfo = {0};
	addInfo.descriptorPool = descriptorPool;
	addInfo.descriptorSets = descriptorSets;
	addInfo.descriptorSetCount = context->imageCount;

	valheim_addDescriptorSet(context, &addInfo, outDescriptorSet);
	return true;
}
