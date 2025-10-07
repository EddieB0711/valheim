#include "valheim.vulkan.rendering.h"
#include "valheim.vulkan.camera.h"
#include "valheim.vulkan.commands.h"
#include "valheim.vulkan.images.h"
#include "valheim.vulkan.swapchain.h"
#include "valheim.vulkan.textures.h"

static void valheim_renderScene(valheim_VulkanContext *context, valheim_VulkanScene *scene, VkCommandBuffer commandBuffer, valheim_VulkanFrameData *frameData) {
	glm_mat4_identity(frameData->model);
	//glm_mat4_mul(frameData->model, scene->localTransform, frameData->model);
	glm_rotate(frameData->model, glm_rad(90.0f), (vec3){1, 0, 0});
	glm_scale(frameData->model, scene->scale);

	if (scene->mesh.indexBuffer > -1) {
		const VkDescriptorSet *descriptorSets = context->descriptorSetManager.descriptorSets.data[scene->descriptorSet];

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineManager.pipelines.data[scene->pipeline]);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineManager.pipelineLayouts.data[scene->pipeline], 0, 1, &descriptorSets[context->currentFrame], 0, NULL);

		vkCmdPushConstants(commandBuffer, context->pipelineManager.pipelineLayouts.data[scene->pipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(*frameData), frameData);

		const VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &context->bufferManager.buffers.data[scene->mesh.vertexBuffer], offsets);

		vkCmdBindIndexBuffer(commandBuffer, context->bufferManager.buffers.data[scene->mesh.indexBuffer], 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, scene->mesh.indexCount, 1, 0, 0, 0);
	}

	for (u32 iChild = 0; iChild < scene->children.length; ++iChild) {
		valheim_renderScene(context, scene->children.data[iChild], commandBuffer, frameData);
	}
}

VkCommandBuffer valheim_beginRendering(valheim_VulkanContext *context) {
	const VkClearValue clearColor = (VkClearValue){.color = (VkClearColorValue){{0.3f, 0.5f, 0.85f, 1.0f}}};
	const VkClearValue clearDepth = (VkClearValue){.depthStencil = (VkClearDepthStencilValue){1.0f, 0}};

	VkRenderingAttachmentInfo colorAttachment = {0};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = context->textureManager.imageViews.data[context->colorTexture];
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
	colorAttachment.resolveImageView = context->swapChainImageViews.data[context->currentFrame];
	colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = clearColor;

	VkRenderingAttachmentInfo depthAttachment = {0};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = context->textureManager.imageViews.data[context->depthTexture];
	depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.clearValue = clearDepth;

	VkRenderingInfo renderingInfo = {0};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea.offset = (VkOffset2D){.x = 0, .y = 0};
	renderingInfo.renderArea.extent = context->capabilities.currentExtent;
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;

	vkCmdBeginRendering(context->commandBuffers.data[context->currentFrame], &renderingInfo);
	return context->commandBuffers.data[context->currentFrame];
}

VkCommandBuffer valheim_endRendering(valheim_VulkanContext *context) {
	VkCommandBuffer commandBuffer = context->commandBuffers.data[context->currentFrame];
	vkCmdEndRendering(commandBuffer);
	return context->commandBuffers.data[context->currentFrame];
}

b8 valheim_beginScene(valheim_VulkanContext *context) {
	VkResult result = vkWaitForFences(context->device, 1, &context->inFlightFences.data[context->currentFrame], VK_TRUE, UINT64_MAX);
	result = vkAcquireNextImageKHR(context->device, context->swapChain, UINT64_MAX, context->imageAvailableSemaphores.data[context->currentFrame], NULL, &context->currentImage);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		valheim_recreateSwapChain(context);
		valheim_destroyTexture(context, context->colorTexture);
		valheim_destroyTexture(context, context->depthTexture);
		valheim_initColorTexture(context);
		valheim_initDepthTexture(context);
		return false;
	}

	result = vkResetFences(context->device, 1, &context->inFlightFences.data[context->currentFrame]);

	if (context->imagesInFlight.data[context->currentImage] != VK_NULL_HANDLE && context->imagesInFlight.data[context->currentImage] != context->inFlightFences.data[context->currentFrame]) {
		result = vkWaitForFences(context->device, 1, &context->imagesInFlight.data[context->currentImage], VK_TRUE, UINT64_MAX);
	}

	context->imagesInFlight.data[context->currentImage] = context->inFlightFences.data[context->currentFrame];

	vkResetCommandBuffer(context->commandBuffers.data[context->currentFrame], 0);

	const VkCommandBuffer commandBuffer = valheim_beginCommandBuffer(context);

	valheim_transitionImageLayout(
		commandBuffer,
		context->swapChainImages.data[context->currentFrame],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	valheim_transitionImageLayout(
		commandBuffer,
		context->textureManager.images.data[context->colorTexture],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	valheim_transitionImageLayout(
		commandBuffer,
		context->textureManager.images.data[context->depthTexture],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		0,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);

	valheim_beginRendering(context);

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (f32)context->capabilities.currentExtent.width;
	viewport.height = (f32)context->capabilities.currentExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};
	scissor.extent = context->capabilities.currentExtent;
	scissor.offset = (VkOffset2D){0, 0};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	f32 aspectRatio = (f32)context->capabilities.currentExtent.width / (f32)context->capabilities.currentExtent.height;

	valheim_VulkanFrameData frameData = {0};
	glm_perspective(glm_rad(45.0f), aspectRatio, 0.01f, 1000.0f, frameData.projection);
	glm_mat4_identity(frameData.model);

	frameData.projection[1][1] *= -1;

	valheim_getCameraView(context, frameData.view);
	valheim_renderScene(context, context->worldScene, commandBuffer, &frameData);
	return true;
}

b8 valheim_endScene(valheim_VulkanContext *context) {
	VkCommandBuffer commandBuffer = valheim_endRendering(context);

	valheim_transitionImageLayout(
		commandBuffer,
		context->swapChainImages.data[context->currentFrame],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		0,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	valheim_endCommandBuffer(context, commandBuffer);

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &context->imageAvailableSemaphores.data[context->currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &context->renderFinishedSemaphores.data[context->currentFrame];

	vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, context->inFlightFences.data[context->currentFrame]);

	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &context->renderFinishedSemaphores.data[context->currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context->swapChain;
	presentInfo.pImageIndices = &context->currentImage;

	VkResult result = vkQueuePresentKHR(context->graphicsQueue, &presentInfo);

	context->currentFrame = (context->currentFrame + 1) % context->imageCount;
	return true;
}
