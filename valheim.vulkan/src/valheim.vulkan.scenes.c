//
// Created by Eddie Boyle on 9/11/2025.
//

#include "valheim.vulkan.scenes.h"

b8 valheim_initVulkanScene(valheim_VulkanContext *context, valheim_Allocator *allocator, valheim_VulkanScene **outScene) {
	valheim_VulkanScene *scene = valheim_allocate(allocator, sizeof(*scene));
	if (!scene) {
		return false;
	}

	valheim_initArray(scene->children, 10, allocator);

	scene->pipeline = -1;
	scene->mesh.indexBuffer = -1;
	scene->mesh.vertexBuffer = -1;
	scene->mesh.material.texture = -1;

	glm_mat4_identity(scene->localTransform);
	glm_mat4_identity(scene->globalTransform);

	glm_vec3_copy((vec3) { 1.0f, 1.0f, 1.0f }, scene->scale);

	*outScene = scene;
	return true;
}
