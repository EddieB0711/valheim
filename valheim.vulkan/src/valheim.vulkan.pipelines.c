#include "valheim.vulkan.pipelines.h"
#include "valheim.vulkan.images.h"
#include "valheim.vulkan.devices.h"

#include <valheim.arena.allocator.h>
#include <valheim.filesystem.h>
#include <valheim.strings.h>

#include <spirv-reflect/spirv_reflect.h>

typedef struct valheim_ShaderStageProperties {
	valheim_IndexableArray(VkPipelineShaderStageCreateInfo) pipelineShaderStages;
	valheim_IndexableArray(VkVertexInputAttributeDescription) vertexInputAttributes;
	valheim_IndexableArray(VkVertexInputBindingDescription) bindingDescriptions;
	valheim_IndexableArray(char *) entries;
	valheim_IndexableArray(VkShaderStageFlags) shaderStages;
	valheim_IndexableArray(VkDescriptorSetLayoutBinding) layoutBindings;
	valheim_IndexableArray(VkPushConstantRange) pushConstants;

	u32 bufferCount[10];
} valheim_ShaderStageProperties;

static u64 valheim_translateFormatToStride(VkFormat format) {
	switch (format) {
	case VK_FORMAT_R32G32B32_SFLOAT:
		return sizeof(vec3);
	case VK_FORMAT_R32G32_SFLOAT:
		return sizeof(vec2);
	}

	return 0;
}

static b8 valheim_initShader(valheim_VulkanContext *context, const char *file, valheim_Allocator *allocator, valheim_ShaderStageProperties *properties, VkShaderModule *outModule) {
	u8 memory[VALHEIM_KIBIBYTE(10)] = {0};

	valheim_ArenaAllocator arena = {0};
	valheim_initArenaAllocator(memory, VALHEIM_ARRAY_LEN(memory), &arena);

	u64 contentLength;
	char *content = valheim_arenaAllocate(&arena, VALHEIM_KIBIBYTE(10));

	if (!valheim_readFile(file, &contentLength, content)) {
		return false;
	}

	SpvReflectShaderModule module = {0};
	spvReflectCreateShaderModule(contentLength, content, &module);

	if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
		u32 stride = 0;

		valheim_initIndexableArray(properties->vertexInputAttributes, module.input_variable_count, allocator);

		for (u32 iInput = 0; iInput < module.input_variable_count; ++iInput) {
			const SpvReflectInterfaceVariable *input = module.input_variables[iInput];

			VkVertexInputAttributeDescription vertexInput = {0};
			vertexInput.location = input->location;
			vertexInput.binding = 0;
			vertexInput.offset = stride;
			vertexInput.format = (VkFormat) input->format;

			stride += valheim_translateFormatToStride((VkFormat) input->format);
			valheim_indexableArrayAppend(properties->vertexInputAttributes, vertexInput);
		}

		valheim_initIndexableArray(properties->bindingDescriptions, 1, allocator);

		VkVertexInputBindingDescription bindingDescription = {0};
		bindingDescription.binding = 0;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		valheim_indexableArrayAppend(properties->bindingDescriptions, bindingDescription);
	}

	valheim_initIndexableArray(properties->entries, module.entry_point_count, allocator);
	valheim_initIndexableArray(properties->shaderStages, module.entry_point_count, allocator);
	valheim_initIndexableArray(properties->pushConstants, module.push_constant_block_count, allocator);
	valheim_initIndexableArray(properties->layoutBindings, module.descriptor_binding_count, allocator);

	VkShaderStageFlags flags[1000] = {0};
	for (u32 iEntry = 0; iEntry < module.entry_point_count; ++iEntry) {
		const SpvReflectEntryPoint *entry = &module.entry_points[iEntry];
		for (u32 iInterface = 0; iInterface < entry->interface_variable_count; ++iInterface) {
			const SpvReflectInterfaceVariable *variable = &entry->interface_variables[iInterface];
			if (variable->storage_class == SpvStorageClassPushConstant) {
				for (u32 iPush = 0; iPush < module.push_constant_block_count; ++iPush) {
					if (module.push_constant_blocks[iPush].spirv_id == variable->spirv_id) {
						flags[module.push_constant_blocks[iPush].spirv_id] |= (VkShaderStageFlags) entry->shader_stage;

						VkPushConstantRange pushConstant = {0};
						pushConstant.size = module.push_constant_blocks[iPush].size;
						pushConstant.stageFlags = flags[module.push_constant_blocks[iPush].spirv_id];
						pushConstant.offset = module.push_constant_blocks[iPush].offset;

						valheim_indexableArrayAppend(properties->pushConstants, pushConstant);
						break;
					}
				}
			} else if (variable->storage_class == SpvStorageClassUniformConstant) {
				for (u32 iDescriptor = 0; iDescriptor < module.descriptor_binding_count; ++iDescriptor) {
					if (variable->spirv_id == module.descriptor_bindings[iDescriptor].spirv_id) {
						VkDescriptorSetLayoutBinding binding = {0};
						binding.binding = module.descriptor_bindings[iDescriptor].binding;
						binding.descriptorType = (VkDescriptorType) module.descriptor_bindings[iDescriptor].descriptor_type;
						binding.stageFlags = entry->shader_stage;
						binding.descriptorCount = 1;

						properties->bufferCount[binding.descriptorType]++;

						for (u32 iArr = 0; iArr < module.descriptor_bindings[iDescriptor].array.dims_count; ++iArr) {
							binding.descriptorCount += module.descriptor_bindings[iDescriptor].array.dims[iArr];
						}

						valheim_indexableArrayAppend(properties->layoutBindings, binding);
					}
				}
			}
		}

		properties->entries.data[iEntry] = valheim_duplicateString(entry->name, allocator);
		properties->entries.length++;

		properties->shaderStages.data[iEntry] = (VkShaderStageFlags) entry->shader_stage;
		properties->shaderStages.length++;
	}

	VkShaderModuleCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = contentLength;
	createInfo.pCode = (const u32 *) content;

	const VkResult result = vkCreateShaderModule(context->device, &createInfo, NULL, outModule);
	valheim_resetArena(&arena);
	return result == VK_SUCCESS;
}

void valheim_initPipelineManager(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	valheim_initIndexableArray(context->pipelineManager.pipelines, 1024, allocator);
	valheim_initIndexableArray(context->pipelineManager.pipelineLayouts, 1024, allocator);
	valheim_initIndexableArray(context->pipelineManager.destriptorSetLayouts, 1024, allocator);
	valheim_initIndexableArray(context->pipelineManager.freeList, 1024, allocator);

	context->pipelineManager.pipelines.length = 1024;
	context->pipelineManager.pipelineLayouts.length = 1024;
	context->pipelineManager.destriptorSetLayouts.length = 1024;
	context->pipelineManager.freeList.length = 1024;

	for (u32 iPipeline = 0; iPipeline < context->pipelineManager.pipelines.length; iPipeline++) {
		context->pipelineManager.freeList.data[iPipeline] = true;
	}
}

void valheim_deinitPipelineManager(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	for (u32 iPipeline = 0; iPipeline < context->pipelineManager.pipelines.length; ++iPipeline) {
		if (!context->pipelineManager.freeList.data[iPipeline]) {
			vkDestroyPipelineLayout(context->device, context->pipelineManager.pipelineLayouts.data[iPipeline], NULL);
			vkDestroyPipeline(context->device, context->pipelineManager.pipelines.data[iPipeline], NULL);
			vkDestroyDescriptorSetLayout(context->device, context->pipelineManager.destriptorSetLayouts.data[iPipeline], NULL);
		}
	}

	valheim_deinitIndexableArray(context->pipelineManager.destriptorSetLayouts);
	valheim_deinitIndexableArray(context->pipelineManager.freeList);
	valheim_deinitIndexableArray(context->pipelineManager.pipelines);
	valheim_deinitIndexableArray(context->pipelineManager.pipelineLayouts);
}

u32 valheim_addPipeline(valheim_VulkanContext *context, valheim_PipelineAddInfo *addInfo) {
	for (u32 iPipeline = 0; iPipeline < context->pipelineManager.pipelines.length; ++iPipeline) {
		if (context->pipelineManager.freeList.data[iPipeline]) {
			context->pipelineManager.freeList.data[iPipeline] = false;
			context->pipelineManager.pipelines.data[iPipeline] = addInfo->pipeline;
			context->pipelineManager.destriptorSetLayouts.data[iPipeline] = addInfo->descriptorSetLayout;
			context->pipelineManager.pipelineLayouts.data[iPipeline] = addInfo->pipelineLayout;
			return iPipeline;
		}
	}

	return 0xFFFFFFF;
}

b8 valheim_initPipelineCache(valheim_VulkanContext *context) {
	VkPipelineCacheCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VkPipelineCache pipelineCache;

	const VkResult result = vkCreatePipelineCache(context->device, &createInfo, NULL, &pipelineCache);
	return result == VK_SUCCESS;
}

b8 valheim_initPipeline(valheim_VulkanContext *context, valheim_PipelineCreateInfo *createInfo, valheim_VulkanPipeline *outPipeline) {
	u8 memory[VALHEIM_KIBIBYTE(10)] = {0};

	valheim_ArenaAllocator arena;
	valheim_initArenaAllocator(memory, VALHEIM_ARRAY_LEN(memory), &arena);

	valheim_Allocator tempAllocator = {0};
	valheim_initAllocatorFromArena(&arena, &tempAllocator);

	valheim_ShaderStageProperties properties = {0};

	VkShaderModule module;
	if (!valheim_initShader(context, createInfo->shaderFile, &tempAllocator, &properties, &module)) {
		return false;
	}

	valheim_initIndexableArray(properties.pipelineShaderStages, properties.entries.length, &tempAllocator);

	for (u32 iName = 0; iName < properties.entries.length; ++iName) {
		VkPipelineShaderStageCreateInfo shaderStage = {0};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.pName = properties.entries.data[iName];
		shaderStage.module = module;
		shaderStage.stage = properties.shaderStages.data[iName];

		valheim_indexableArrayAppend(properties.pipelineShaderStages, shaderStage);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (u32) properties.layoutBindings.length;
	layoutInfo.pBindings = properties.layoutBindings.data;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkResult result = vkCreateDescriptorSetLayout(context->device, &layoutInfo, NULL, &descriptorSetLayout);
	if (result != VK_SUCCESS) {
		return false;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = (u32) properties.pushConstants.length;
	pipelineLayoutCreateInfo.pPushConstantRanges = properties.pushConstants.data;

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(context->device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
	if (result != VK_SUCCESS) {
		return false;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState = {0};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexAttributeDescriptionCount = (u32) properties.vertexInputAttributes.length;
	vertexInputState.pVertexAttributeDescriptions = properties.vertexInputAttributes.data;
	vertexInputState.vertexBindingDescriptionCount = (u32) properties.bindingDescriptions.length;
	vertexInputState.pVertexBindingDescriptions = properties.bindingDescriptions.data;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewportState = {0};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {0};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampleState = {0};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_TRUE;
	multisampleState.minSampleShading = 0.2f;
	multisampleState.rasterizationSamples = valheim_getMaxSampleCount(context);

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {0};
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {0};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = VALHEIM_ARRAY_LEN(dynamicStates);
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {0};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_NEVER;
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	VkPipelineRenderingCreateInfo renderingState = {0};
	renderingState.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingState.colorAttachmentCount = 1;
	renderingState.pColorAttachmentFormats = &context->format.format;
	renderingState.depthAttachmentFormat = valheim_getDepthFormat(context);

	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = &renderingState;
	pipelineInfo.stageCount = (u32) properties.pipelineShaderStages.length;
	pipelineInfo.pStages = properties.pipelineShaderStages.data;
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;

	VkPipeline pipeline;
	result = vkCreateGraphicsPipelines(context->device, NULL, 1, &pipelineInfo, NULL, &pipeline);

	if (result != VK_SUCCESS) {
		return false;
	}

	vkDestroyShaderModule(context->device, module, NULL);

	valheim_PipelineAddInfo addInfo = {0};
	addInfo.descriptorSetLayout = descriptorSetLayout;
	addInfo.pipelineLayout = pipelineLayout;
	addInfo.pipeline = pipeline;

	*outPipeline = (s32) valheim_addPipeline(context, &addInfo);
	return true;
}

b8 valheim_loadPipelines(valheim_VulkanContext *context, valheim_Allocator *allocator) {
	const char *files[] = {
		"assets/shaders/Builtin.StaticTexturedMesh.spv",
		"assets/shaders/BuiltIn.ImGui.spv",
	};

	const char *names[] = {
		VALHEIM_STATIC_TEXTURED_MESH,
		VALHEIM_IMGUI_PIPELINE,
	};

	for (u32 iFile = 0; iFile < VALHEIM_ARRAY_LEN(files); ++iFile) {
		valheim_PipelineCreateInfo pipelineInfo = {.shaderFile = files[iFile]};
		valheim_VulkanPipeline pipeline;
		if (!valheim_initPipeline(context, &pipelineInfo, &pipeline)) {
			return false;
		}

		valheim_mapInsert(&context->pipelineHandles, names[iFile], valheim_stringLength(names[iFile]), &pipeline, allocator);
	}

	return true;
}
