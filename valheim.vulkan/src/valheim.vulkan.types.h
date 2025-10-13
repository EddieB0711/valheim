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

typedef b8( *valheim_RunJob )( void *, valheim_Allocator * );
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
	valheim_IndexableArray( VkBuffer ) buffers;
	valheim_IndexableArray( VkDeviceMemory ) bufferMemory;
	valheim_IndexableArray( VkDeviceSize ) bufferSizes;
	valheim_IndexableArray( b8 ) buffersInUse;
} valheim_VulkanBufferManager;

typedef struct valheim_VulkanTextureManager {
	valheim_IndexableArray( VkImage ) images;
	valheim_IndexableArray( VkImageView ) imageViews;
	valheim_IndexableArray( VkSampler ) samplers;
	valheim_IndexableArray( VkDeviceMemory ) imageMemory;
	valheim_IndexableArray( VkDeviceSize ) imageSizes;
	valheim_IndexableArray( b8 ) freeList;
} valheim_VulkanTextureManager;

typedef struct valheim_VulkanPipelineManager {
	valheim_IndexableArray( VkPipelineLayout ) pipelineLayouts;
	valheim_IndexableArray( VkPipeline ) pipelines;
	valheim_IndexableArray( VkDescriptorSetLayout ) destriptorSetLayouts;
	valheim_IndexableArray( b8 ) freeList;
} valheim_VulkanPipelineManager;

typedef struct valheim_DescriptorSetManager {
	valheim_IndexableArray( VkDescriptorPool ) descriptorPools;
	valheim_IndexableArray( VkDescriptorSet * ) descriptorSets;
	valheim_IndexableArray( u32 ) descriptorSetCounts;
	valheim_IndexableArray( b8 ) freeList;
} valheim_DescriptorSetManager;

typedef struct valheim_VulkanMaterial {
	f32 albedo;
	valheim_VulkanTexture texture;
} valheim_VulkanMaterial;

typedef struct valheim_VulkanMesh {
	valheim_VulkanBuffer vertexBuffer;
	valheim_VulkanBuffer indexBuffer;
	valheim_VulkanMaterial material;
	valheim_VulkanPipeline pipelineHandle;
	valheim_VulkanDescriptorSet descriptorSetHandle;
	u32 indexCount;
} valheim_VulkanMesh;

typedef struct valheim_VulkanSceneHeirarchy {
	s32 parent;
	s32 firstChild;
	s32 nextSibling;
	s32 lastSibling;
	s32 depth;
} valheim_VulkanSceneHeirarchy;

typedef struct valheim_VulkanScene {
	valheim_IndexableArray( valheim_VulkanSceneHeirarchy ) heirarchies;
	valheim_Map nodeMeshes;
	valheim_Map nodeMaterials;
	valheim_Array localTransforms;
	valheim_Array globalTransforms;
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
	valheim_Allocator *allocator;
} valheim_VulkanJob;

typedef struct valheim_VulkanJobManager {
	valheim_IndexableArray( valheim_VulkanJob ) jobs;
} valheim_VulkanJobManager;

typedef struct valheim_VulkanInitializationGraphNode {
	u32 id;
	const char *name;
	valheim_IndexableArray( const char * ) inDegrees;
	valheim_IndexableArray( const char * ) outDegrees;
	b8( *initialize )( struct valheim_VulkanContext *, valheim_Allocator * );
	b8 isInitialized;
} valheim_VulkanInitializationGraphNode;

typedef struct valheim_VulkanInitializationGraph {
	valheim_Map nodes;
	valheim_IndexableArray( valheim_VulkanInitializationGraphNode * ) arrNodes;
	valheim_IndexableArray( valheim_VulkanInitializationGraphNode * ) sortedNodes;
} valheim_VulkanInitializationGraph;

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

	valheim_IndexableArray( VkImage ) swapChainImages;
	valheim_IndexableArray( VkImageView ) swapChainImageViews;
	valheim_IndexableArray( VkCommandBuffer ) commandBuffers;
	valheim_IndexableArray( VkSemaphore ) imageAvailableSemaphores;
	valheim_IndexableArray( VkSemaphore ) renderFinishedSemaphores;
	valheim_IndexableArray( VkFence ) inFlightFences;
	valheim_IndexableArray( VkFence ) imagesInFlight;

	valheim_Map pipelineHandles;

	valheim_VulkanCamera camera;
	valheim_VulkanBufferManager bufferManager;
	valheim_VulkanTextureManager textureManager;
	valheim_VulkanPipelineManager pipelineManager;
	valheim_DescriptorSetManager descriptorSetManager;
	valheim_VulkanScene worldScene;
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
