//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.vulkan.instance.h"

b8 valheim_initInstance(valheim_VulkanContext *context) {
	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Valheim";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Valheim";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_4;

	const char *extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const char *Layers[] = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = VALHEIM_ARRAY_LEN(extensions);
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledLayerCount = VALHEIM_ARRAY_LEN(Layers);
	createInfo.ppEnabledLayerNames = Layers;

	const VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);

	if (result == VK_SUCCESS) {
		volkLoadInstance(context->instance);
		return true;
	}

	return false;
}

void valheim_deinitInstance(valheim_VulkanContext *context) {
	if (context->instance) {
		if (context->surface) {
			vkDestroySurfaceKHR(context->instance, context->surface, NULL);
		}

		vkDestroyInstance(context->instance, NULL);
	}
}
