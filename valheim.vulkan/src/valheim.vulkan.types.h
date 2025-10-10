//
// Created by Eddie Boyle on 9/9/2025.
//

#ifndef VALHEIM_VALHEIM_VULKAN_TYPES_H
#define VALHEIM_VALHEIM_VULKAN_TYPES_H

#include <volk.h>

#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>

#include <GLFW/glfw3.h>

#include <valheim.array.h>
#include <valheim.map.h>
#include <valheim.queue.h>

#define VALHEIM_STATIC_TEXTURED_MESH "StaticTexturedMesh"
#define VALHEIM_IMGUI_PIPELINE "ImGuiPipeline"

struct valheim_VulkanContext;
struct valheim_VulkanScene;

typedef b8( *valheim_RunJob )( void * );
typedef void ( *valheim_RenderScene )( struct valheim_VulkanContext *, struct valheim_VulkanScene * );

typedef s32 valheim_Handle;
typedef valheim_Handle valheim_VulkanBuffer;
typedef valheim_Handle valheim_VulkanTexture;
typedef valheim_Handle valheim_VulkanPipeline;
typedef valheim_Handle valheim_VulkanDescriptorSet;

typedef struct valheim_VulkanFrameData {
	mat4 projection;
	mat4 view;
	mat4 model;
} valheim_VulkanFrameData;

typedef struct valheim_VulkanBufferManager {
	valheim_Array( VkBuffer ) buffers;
	valheim_Array( VkDeviceMemory ) bufferMemory;
	valheim_Array( VkDeviceSize ) bufferSizes;
	valheim_Array( b8 ) buffersInUse;
} valheim_VulkanBufferManager;

typedef struct valheim_VulkanTextureManager {
	valheim_Array( VkImage ) images;
	valheim_Array( VkImageView ) imageViews;
	valheim_Array( VkSampler ) samplers;
	valheim_Array( VkDeviceMemory ) imageMemory;
	valheim_Array( VkDeviceSize ) imageSizes;
	valheim_Array( b8 ) freeList;
} valheim_VulkanTextureManager;

typedef struct valheim_VulkanPipelineManager {
	valheim_Array( VkPipelineLayout ) pipelineLayouts;
	valheim_Array( VkPipeline ) pipelines;
	valheim_Array( VkDescriptorSetLayout ) destriptorSetLayouts;
	valheim_Array( b8 ) freeList;
} valheim_VulkanPipelineManager;

typedef struct valheim_DescriptorSetManager {
	valheim_Array( VkDescriptorPool ) descriptorPools;
	valheim_Array( VkDescriptorSet * ) descriptorSets;
	valheim_Array( u32 ) descriptorSetCounts;
	valheim_Array( b8 ) freeList;
} valheim_DescriptorSetManager;

typedef struct valheim_VulkanMaterial {
	f32 albedo;
	valheim_VulkanTexture texture;
} valheim_VulkanMaterial;

typedef struct valheim_VulkanMesh {
	valheim_VulkanBuffer vertexBuffer;
	valheim_VulkanBuffer indexBuffer;
	valheim_VulkanMaterial material;

	u32 indexCount;
} valheim_VulkanMesh;

typedef struct valheim_VulkanScene {
	vec3 scale;

	valheim_VulkanMesh mesh;
	valheim_VulkanPipeline pipeline;
	valheim_VulkanDescriptorSet descriptorSet;
	valheim_RenderScene render;

	mat4 localTransform;
	mat4 globalTransform;

	valheim_Array( struct valheim_VulkanScene * ) children;
} valheim_VulkanScene;

typedef struct valheim_VulkanCamera {
	vec3 position;
	vec3 front;
	vec3 up;
	vec3 worldUp;
	vec3 right;
	f32 yaw;
	f32 pitch;
} valheim_VulkanCamera;

typedef struct valheim_VulkanJob {
	valheim_RunJob execute;
	void *params;
} valheim_VulkanJob;

typedef struct valheim_VulkanJobManager {
	valheim_Array( valheim_VulkanJob ) jobs;
} valheim_VulkanJobManager;

typedef struct valheim_UiInput {
	b8 shouldShowUI;
	b8 shouldClose;
} valheim_UiInput;

typedef struct valheim_VulkanContext {
	GLFWwindow *window;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	VkCommandPool commandPool;

	valheim_Array( VkImage ) swapChainImages;
	valheim_Array( VkImageView ) swapChainImageViews;
	valheim_Array( VkCommandBuffer ) commandBuffers;
	valheim_Array( VkSemaphore ) imageAvailableSemaphores;
	valheim_Array( VkSemaphore ) renderFinishedSemaphores;
	valheim_Array( VkFence ) inFlightFences;
	valheim_Array( VkFence ) imagesInFlight;

	valheim_Map( const char *, valheim_VulkanPipeline ) pipelineHandles;

	valheim_VulkanCamera camera;
	valheim_VulkanBufferManager bufferManager;
	valheim_VulkanTextureManager textureManager;
	valheim_VulkanPipelineManager pipelineManager;
	valheim_DescriptorSetManager descriptorSetManager;
	valheim_VulkanScene *worldScene;
	valheim_VulkanBuffer frameDataBuffer;

	valheim_UiInput uiInput;

	u32 imageCount;
	u32 currentImage;
	u32 currentFrame;

	s32 graphicsQueueIndex;
	s32 presentQueueIndex;

	b8 firstMouseMoved;

	f32 lastMouseX;
	f32 lastMouseY;

	valheim_VulkanTexture depthTexture;
	valheim_VulkanTexture colorTexture;

	valheim_Queue highPriority;
	valheim_Queue mediumPriority;
	valheim_Queue lowPriority;
} valheim_VulkanContext;

#endif //VALHEIM_VALHEIM_VULKAN_TYPES_H
