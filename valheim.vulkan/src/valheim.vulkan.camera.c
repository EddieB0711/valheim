#include "valheim.vulkan.camera.h"

#define VELOCITY 1.5f

static void valheim_updateCamera(valheim_VulkanContext *context) {
	vec3 front = {0};
	front[0] = cosf(glm_rad(context->camera.yaw)) * cosf(glm_rad(context->camera.pitch));
	front[1] = sinf(glm_rad(context->camera.pitch));
	front[2] = sinf(glm_rad(context->camera.yaw)) * cosf(glm_rad(context->camera.pitch));

	glm_normalize_to(front, context->camera.front);

	vec3 cross;
	glm_cross(context->camera.front, context->camera.worldUp, cross);
	glm_normalize_to(cross, context->camera.right);

	glm_cross(context->camera.right, context->camera.front, cross);
	glm_normalize_to(cross, context->camera.up);
}

void valheim_initCamera(valheim_VulkanContext *context) {
	glm_vec3_copy((vec3) { 0.0f, 0.0f, -2.0f }, context->camera.position);
	glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, context->camera.up);
	glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, context->camera.worldUp);

	context->camera.yaw = 90.0f;
	context->camera.pitch = 0.0f;

	valheim_updateCamera(context);
}

void valheim_cameraMoveForward(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.front, velocity, motion);
	glm_vec3_add(context->camera.position, motion, context->camera.position);
}

void valheim_cameraMoveBackward(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.front, velocity, motion);
	glm_vec3_sub(context->camera.position, motion, context->camera.position);
}

void valheim_cameraMoveLeft(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.right, velocity, motion);
	glm_vec3_sub(context->camera.position, motion, context->camera.position);
}

void valheim_cameraMoveRight(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.right, velocity, motion);
	glm_vec3_add(context->camera.position, motion, context->camera.position);
}

void valheim_cameraMoveUp(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.up, velocity, motion);
	glm_vec3_add(context->camera.position, motion, context->camera.position);
}

void valheim_cameraMoveDown(valheim_VulkanContext *context, f32 deltaTime) {
	const f32 velocity = VELOCITY * deltaTime;

	vec3 motion;
	glm_vec3_scale(context->camera.up, velocity, motion);
	glm_vec3_sub(context->camera.position, motion, context->camera.position);
}

void valheim_cameraRotate(valheim_VulkanContext *context, f32 xoffset, f32 yoffset, f32 sensitivity) {
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	context->camera.yaw += xoffset;
	context->camera.pitch += yoffset;

	if (context->camera.pitch > 89.0f) {
		context->camera.pitch = 89.0f;
	} else if (context->camera.pitch < -89.0f) {
		context->camera.pitch = -89.0f;
	}

	valheim_updateCamera(context);
}

void valheim_getCameraView(valheim_VulkanContext *context, mat4 view) {
	vec3 center;
	glm_vec3_add(context->camera.position, context->camera.front, center);
	glm_lookat(context->camera.position, center, context->camera.up, view);
}
