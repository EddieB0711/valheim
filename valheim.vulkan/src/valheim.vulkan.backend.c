#include "valheim.vulkan.backend.h"
#include "valheim.vulkan.buffers.h"
#include "valheim.vulkan.instance.h"
#include "valheim.vulkan.devices.h"
#include "valheim.vulkan.swapchain.h"
#include "valheim.vulkan.commands.h"
#include "valheim.vulkan.descriptors.h"
#include "valheim.vulkan.textures.h"
#include "valheim.vulkan.pipelines.h"
#include "valheim.vulkan.gltf.h"
#include "valheim.vulkan.scenes.h"
#include "valheim.vulkan.camera.h"
#include "valheim.vulkan.rendering.h"
#include "valheim.vulkan.sync.h"

#include <valheim.hashing.h>
#include <valheim.strings.h>
#include <valheim.thread.h>

#include <Windows.h>

typedef struct valheim_VulkanInitializationGraphNode {
	u32 id;
	const char *name;
	valheim_Array(const char *) inDegrees;
	valheim_Array(const char *) outDegrees;
} valheim_VulkanInitializationGraphNode;

typedef struct valheim_VulkanInitializationGraph {
	valheim_Array(valheim_VulkanInitializationGraphNode *) nodes;
} valheim_VulkanInitializationGraph;

typedef struct valheim_QueueProcessingBeginInfo {
	valheim_VulkanContext *context;
	valheim_Queue *queue;
} valheim_QueueProcessingBeginInfo;

static void valheim_addInitializationStep(valheim_VulkanInitializationGraph *graph, const char *name, const char **inDegrees, u32 inDegreeCount, const char **outDegrees, u32 outDegreeCount, valheim_Allocator *allocator) {
	valheim_VulkanInitializationGraphNode *node = valheim_allocate(allocator, sizeof(*node));
	node->name = name;

	if (inDegreeCount > 0) {
		valheim_initArray(node->inDegrees, inDegreeCount, allocator);
	}

	if (outDegreeCount > 0) {
		valheim_initArray(node->outDegrees, outDegreeCount, allocator);
	}

	for (u32 iDeg = 0; iDeg < inDegreeCount; ++iDeg) {
		valheim_arrayAppend(node->inDegrees, inDegrees[iDeg]);
	}

	for (u32 iDeg = 0; iDeg < outDegreeCount; ++iDeg) {
		valheim_arrayAppend(node->outDegrees, outDegrees[iDeg]);
	}

	valheim_arrayAppend(graph->nodes, node);
}

static void valheim_sortInitializationSteps(valheim_VulkanInitializationGraph *graph, valheim_Allocator *allocator) {
	valheim_Queue queue;
	valheim_initQueue(sizeof(valheim_VulkanInitializationGraphNode *), allocator, &queue);

	valheim_Map(const char *, u32) lengths;
	valheim_initMap(lengths, 100, valheim_createHash, allocator);

	for (u32 iNode = 0; iNode < graph->nodes.length; ++iNode) {
		valheim_VulkanInitializationGraphNode *node = graph->nodes.data[iNode];

		if (node->inDegrees.length == 0) {
			valheim_queueEnqueue(&queue, &node, allocator);
		} else {
			valheim_mapInsert(lengths, node->name, valheim_stringLength(node->name), node->outDegrees.length);
		}
	}

	valheim_Array(valheim_VulkanInitializationGraphNode *) sorted;
	valheim_initArray(sorted, graph->nodes.length, allocator);

	valheim_VulkanInitializationGraphNode *node;
	while (valheim_queueDequeue(&queue, allocator, &node)) {
		valheim_arrayAppend(sorted, node);

		for (u32 iOut = 0; iOut < node->outDegrees.length; ++iOut) {
			const char *name = node->outDegrees.data[iOut];

			u32 length = 0;
			valheim_mapFind(lengths, name, valheim_stringLength(name), length);

			if (--length == 0) {
				valheim_VulkanInitializationGraphNode *outNode;
				for (u32 iNode = 0; iNode < graph->nodes.length; ++iNode) {
					if (valheim_stringsEqual(graph->nodes.data[iNode]->name, name)) {
						valheim_queueEnqueue(&queue, &graph->nodes.data[iNode], allocator);
						break;
					}
				}
			} else {
				valheim_mapInsert(lengths, name, valheim_stringLength(name), length);
			}
		}
	}

	valheim_arrayClear(graph->nodes);

	for (u32 iNode = 0; iNode < sorted.length; ++iNode) {
		valheim_arrayAppend(graph->nodes, sorted.data[iNode]);
	}

	valheim_deinitMap(lengths);
	valheim_deinitArray(sorted);
	valheim_deinitQueue(&queue, allocator);
}

static b8 valheim_loadPipelinesJob(void *params) {
	valheim_VulkanContext *context = params;
	return false;
}

static b8 valheim_loadGltfFileJob(void *args) {
	valheim_VulkanContext *context = args;

	valheim_VulkanScene *terrainScene;
	valheim_loadGltfFile(context, "assets/terrain.gltf", context->allocator, &terrainScene);
	valheim_arrayAppend(context->worldScene->children, terrainScene);
	return true;
}

static void valheim_beginQueueProcessing(valheim_QueueProcessingBeginInfo *beginInfo) {
	LPVOID mainFiber = ConvertThreadToFiber(NULL);

	while (true) {
		valheim_VulkanJob job = {0};
		while (valheim_queueDequeue(beginInfo->queue, beginInfo->context->allocator, &job)) {
			if (job.execute) {
				job.execute(job.params);
			}
		}
	}

	ConvertFiberToThread();
}

static void valheim_processInput(valheim_VulkanContext *context, f32 deltaTime) {
	if (glfwGetKey(context->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(context->window, true);
	}

	if (glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS) {
		valheim_cameraMoveForward(context, deltaTime);
	}
	if (glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS) {
		valheim_cameraMoveBackward(context, deltaTime);
	}
	if (glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS) {
		valheim_cameraMoveLeft(context, deltaTime);
	}
	if (glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS) {
		valheim_cameraMoveRight(context, deltaTime);
	}
	if (glfwGetKey(context->window, GLFW_KEY_Q) == GLFW_PRESS) {
		valheim_cameraMoveDown(context, deltaTime);
	}
	if (glfwGetKey(context->window, GLFW_KEY_E) == GLFW_PRESS) {
		valheim_cameraMoveUp(context, deltaTime);
	}

	if (glfwGetKey(context->window, GLFW_KEY_P) == GLFW_PRESS) {
		printf("camera position: %f, %f, %f\n", context->camera.position[0], context->camera.position[1], context->camera.position[2]);
	}
}

static void valheim_processMouseMovement(GLFWwindow *window, f64 xpos, f64 ypos) {
	valheim_VulkanContext *context = glfwGetWindowUserPointer(window);

	const f32 fxpos = (f32)xpos;
	const f32 fypos = (f32)ypos;

	if (context->firstMouseMoved) {
		context->lastMouseX = fxpos;
		context->lastMouseY = fypos;
		context->firstMouseMoved = false;
	}

	const f32 xoffset = fxpos - context->lastMouseX;
	const f32 yoffset = context->lastMouseY - fypos;

	context->lastMouseX = fxpos;
	context->lastMouseY = fypos;

	valheim_cameraRotate(context, xoffset, yoffset, 0.1f);
}

b8 valheim_initContext(valheim_Allocator *allocator, valheim_VulkanContext *context) {
	valheim_zeroMemory(context, sizeof(*context));
	context->allocator = allocator;
	context->firstMouseMoved = true;

#ifdef VALIDATE
#undef VALIDATE
#endif

#define VALIDATE(result) if (result) {} else { valheim_deinitContext(context); return false; }

	VALIDATE(glfwInit() == GLFW_TRUE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	valheim_initBufferManager(context);
	valheim_initTextureManager(context);
	valheim_initPipelineManager(context);
	valheim_initDescriptorSetManager(context);
	valheim_initCamera(context);

	valheim_initQueue(sizeof(valheim_VulkanJob), allocator, &context->highPriority);
	valheim_initQueue(sizeof(valheim_VulkanJob), allocator, &context->mediumPriority);
	valheim_initQueue(sizeof(valheim_VulkanJob), allocator, &context->lowPriority);

	valheim_initMap(context->pipelineHandles, 1024, valheim_createHash, allocator);

	VALIDATE((context->window = glfwCreateWindow(1280, 720, "Graph Visualizer", NULL, NULL)));
	glfwSetWindowUserPointer(context->window, context);
	glfwSetCursorPosCallback(context->window, valheim_processMouseMovement);
	glfwSetCursorPos(context->window, 1280.0f / 2, 720.0f / 2);
	glfwSetInputMode(context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//valheim_VulkanInitializationGraph depGraph = {0};
	//valheim_initArray(depGraph.nodes, 10, allocator);

	//valheim_addInitializationStep(&depGraph, "create-instance", NULL, 0, (const char *[]) { "create-surface" }, 1, allocator);
	//valheim_addInitializationStep(&depGraph, "create-surface", (const char *[]) { "create-instance" }, 1, (const char *[]) { "select-physical-device" }, 1, allocator);
	//valheim_addInitializationStep(&depGraph, "select-physical-device", (const char *[]) { "create-surface" }, 1, (const char *[]) { "create-logical-device" }, 1, allocator);
	//valheim_addInitializationStep(&depGraph, "create-logical-device", (const char *[]) { "select-physical-device" }, 1, (const char *[]) { "create-swap-chain", "create-command-pool", "create-depth-texture", "create-color-texture", "create-uniform-buffer", "load-pipelines" }, 6, allocator);
	//valheim_addInitializationStep(&depGraph, "create-swap-chain", (const char *[]) { "create-logical-device" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "create-command-pool", (const char *[]) { "create-logical-device" }, 1, (const char *[]) { "allocate-command-buffers" }, 1, allocator);
	//valheim_addInitializationStep(&depGraph, "allocate-command-buffers", (const char *[]) { "create-command-pool" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "create-depth-texture", (const char *[]) { "create-logical-device" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "create-color-texture", (const char *[]) { "create-logical-device" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "create-uniform-buffer", (const char *[]) { "create-logical-device" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "load-pipelines", (const char *[]) { "create-logical-device" }, 1, NULL, 0, allocator);
	//valheim_addInitializationStep(&depGraph, "create-scene", (const char *[]) { "load-pipelines" }, 1, NULL, 0, allocator);

	//valheim_sortInitializationSteps(&depGraph, allocator);

	VALIDATE(volkInitialize() == VK_SUCCESS);
	VALIDATE(valheim_initInstance(context));
	VALIDATE(glfwCreateWindowSurface(context->instance, context->window, NULL, &context->surface) == VK_SUCCESS);
	VALIDATE(valheim_selectPhysicalDevice(context));
	VALIDATE(valheim_initLogicalDevice(context));
	VALIDATE(valheim_initSwapChain(context));
	VALIDATE(valheim_initSyncObjects(context, allocator));
	VALIDATE(valheim_initCommandPool(context));
	VALIDATE(valheim_acquireCommandBuffers(context));
	VALIDATE(valheim_initDepthTexture(context));
	VALIDATE(valheim_initColorTexture(context));
	//VALIDATE(valheim_initUniformBuffer(context, sizeof(valheim_VulkanFrameData), &context->frameDataBuffer));
	VALIDATE(valheim_loadPipelines(context));
	VALIDATE(valheim_initVulkanScene(context, allocator, &context->worldScene));

	valheim_VulkanScene *testScene;
	VALIDATE(valheim_loadGltfFile(context, "assets/terrain.gltf", allocator, &testScene));
	valheim_arrayAppend(context->worldScene->children, testScene);

#undef VALIDATE

	//valheim_QueueProcessingBeginInfo beginInfo = {0};
	//beginInfo.context = context;
	//beginInfo.queue = &context->highPriority;

	//valheim_ThreadCreateInfo threadCreateInfo = {0};
	//threadCreateInfo.ownsParams = true;
	//threadCreateInfo.params = &beginInfo;
	//threadCreateInfo.entry = valheim_beginQueueProcessing;
	//threadCreateInfo.paramsSize = sizeof(valheim_QueueProcessingBeginInfo);

	//valheim_Thread queueThread;
	//valheim_createThread(&threadCreateInfo, allocator, &queueThread);

	//valheim_VulkanJob testJob = {0};
	//testJob.execute = valheim_loadGltfFileJob;
	//testJob.params = context;

	//valheim_queueEnqueue(&context->highPriority, &testJob, allocator);
	return true;
}

void valheim_deinitContext(valheim_VulkanContext *context) {
	if (context->device) {
		vkDeviceWaitIdle(context->device);
	}

	for (u32 iImage = 0; iImage < context->imageCount; ++iImage) {
		vkDestroySemaphore(context->device, context->imageAvailableSemaphores.data[iImage], NULL);
		vkDestroySemaphore(context->device, context->renderFinishedSemaphores.data[iImage], NULL);
		vkDestroyFence(context->device, context->inFlightFences.data[iImage], NULL);
		vkDestroyImageView(context->device, context->swapChainImageViews.data[iImage], NULL);
	}

	valheim_deinitTextureManager(context);
	valheim_deinitBufferManager(context);
	valheim_deinitDescriptorSetManager(context);
	valheim_deinitPipelineManager(context);
	valheim_releaseCommandBuffers(context);
	valheim_deinitCommandPool(context);
	valheim_deinitSwapChain(context);
	valheim_deinitLogicalDevice(context);
	valheim_deinitInstance(context);

	volkFinalize();

	glfwDestroyWindow(context->window);
	glfwTerminate();
}

void valheim_runApplication(valheim_VulkanContext *context) {
	f32 lastFrame = 0.0f;
	while (!glfwWindowShouldClose(context->window)) {
		const f32 currentFrame = (f32)glfwGetTime();
		const f32 deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		valheim_processInput(context, deltaTime);
		if (valheim_beginScene(context)) {
			valheim_endScene(context);
		}
	}
}
