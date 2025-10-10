//
// Created by Eddie Boyle on 9/9/2025.
//

#include "valheim.vulkan.devices.h"

#include <valheim.arena.allocator.h>

u32 valheim_getMemoryTypeIndex( valheim_VulkanContext *context, u32 typeFilter, VkMemoryPropertyFlags properties ) {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties( context->physicalDevice, &memoryProperties );

	for ( u32 iProps = 0; iProps < memoryProperties.memoryTypeCount; iProps++ ) {
		if ( typeFilter & ( 1 << iProps ) && ( memoryProperties.memoryTypes[ iProps ].propertyFlags & properties ) == properties ) {
			return iProps;
		}
	}

	return 0;
}

b8 valheim_selectPhysicalDevice( valheim_VulkanContext *context ) {
	u8 memory[ VALHEIM_KIBIBYTE( 1 ) ] = { 0 };

	valheim_ArenaAllocator allocator;
	valheim_initArenaAllocator( memory, VALHEIM_ARRAY_LEN( memory ), &allocator );

	u32 count = 0;
	vkEnumeratePhysicalDevices( context->instance, &count, NULL );

	VkPhysicalDevice *physicalDevices = valheim_arenaAllocate( &allocator, count * sizeof( VkPhysicalDevice ) );
	vkEnumeratePhysicalDevices( context->instance, &count, physicalDevices );

	for ( u32 iDevice = 0; iDevice < count; ++iDevice ) {
		VkPhysicalDevice device = physicalDevices[ iDevice ];

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties( device, &properties );

		if ( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
			u32 familyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties( device, &familyCount, NULL );

			VkQueueFamilyProperties *queueFamilies = valheim_arenaAllocate( &allocator, familyCount * sizeof( VkQueueFamilyProperties ) );
			vkGetPhysicalDeviceQueueFamilyProperties( device, &familyCount, queueFamilies );

			for ( u32 iQueue = 0; iQueue < familyCount; ++iQueue ) {
				VkQueueFamilyProperties *family = &queueFamilies[ iQueue ];

				if ( family->queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
					context->graphicsQueueIndex = ( s32 ) iQueue;
				}

				VkBool32 presentSupported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR( device, iQueue, context->surface, &presentSupported );

				if ( presentSupported ) {
					context->presentQueueIndex = ( s32 ) iQueue;
				}

				if ( context->graphicsQueueIndex > -1 && context->presentQueueIndex > -1 ) {
					break;
				}
			}

			context->physicalDevice = device;
			break;
		}
	}

	return context->physicalDevice != VK_NULL_HANDLE;
}

b8 valheim_initLogicalDevice( valheim_VulkanContext *context ) {
	f32 priority = 1.0f;

	u32 queueCount = 0;

	VkDeviceQueueCreateInfo queueCreateInfo[ 2 ] = { 0 };
	queueCreateInfo[ queueCount ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo[ queueCount ].queueFamilyIndex = context->graphicsQueueIndex;
	queueCreateInfo[ queueCount ].queueCount = 1;
	queueCreateInfo[ queueCount++ ].pQueuePriorities = &priority;

	if ( context->presentQueueIndex != context->graphicsQueueIndex ) {
		queueCreateInfo[ queueCount ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo[ queueCount ].queueFamilyIndex = context->presentQueueIndex;
		queueCreateInfo[ queueCount ].queueCount = 1;
		queueCreateInfo[ queueCount++ ].pQueuePriorities = &priority;
	}

	const char *extensions[] = {
		 VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkPhysicalDeviceVulkan12Features vulkan12 = { 0 };
	vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vulkan12.descriptorBindingPartiallyBound = VK_TRUE;
	vulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
	vulkan12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	vulkan12.descriptorBindingVariableDescriptorCount = VK_TRUE;
	vulkan12.descriptorIndexing = VK_TRUE;
	vulkan12.runtimeDescriptorArray = VK_TRUE;

	VkPhysicalDeviceVulkan13Features vulkan13 = { 0 };
	vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan13.pNext = &vulkan12;
	vulkan13.dynamicRendering = VK_TRUE;
	vulkan13.synchronization2 = VK_TRUE;

	VkPhysicalDeviceFeatures2 features2 = { 0 };
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	vkGetPhysicalDeviceFeatures2( context->physicalDevice, &features2 );

	features2.pNext = &vulkan13;
	features2.features.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = { 0 };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = &features2;
	deviceCreateInfo.queueCreateInfoCount = queueCount;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = VALHEIM_ARRAY_LEN( extensions );
	deviceCreateInfo.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateDevice( context->physicalDevice, &deviceCreateInfo, NULL, &context->device );

	if ( result == VK_SUCCESS ) {
		volkLoadDevice( context->device );

		vkGetDeviceQueue( context->device, context->graphicsQueueIndex, 0, &context->graphicsQueue );
		vkGetDeviceQueue( context->device, context->presentQueueIndex, 0, &context->presentQueue );

		return true;
	}

	return false;
}

void valheim_deinitLogicalDevice( valheim_VulkanContext *context ) {
	if ( context->device != VK_NULL_HANDLE ) {
		vkDestroyDevice( context->device, NULL );
	}
}

VkSampleCountFlagBits valheim_getMaxSampleCount( valheim_VulkanContext *context ) {
	VkPhysicalDeviceProperties properties = { 0 };
	vkGetPhysicalDeviceProperties( context->physicalDevice, &properties );

	const VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;

	if ( counts & VK_SAMPLE_COUNT_64_BIT ) { return VK_SAMPLE_COUNT_64_BIT; }
	if ( counts & VK_SAMPLE_COUNT_32_BIT ) { return VK_SAMPLE_COUNT_32_BIT; }
	if ( counts & VK_SAMPLE_COUNT_16_BIT ) { return VK_SAMPLE_COUNT_16_BIT; }
	if ( counts & VK_SAMPLE_COUNT_8_BIT ) { return VK_SAMPLE_COUNT_8_BIT; }
	if ( counts & VK_SAMPLE_COUNT_4_BIT ) { return VK_SAMPLE_COUNT_4_BIT; }
	if ( counts & VK_SAMPLE_COUNT_2_BIT ) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}