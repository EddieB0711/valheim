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
#include "valheim.vulkan.jobs.h"

#include <valheim.hashing.h>
#include <valheim.strings.h>
#include <valheim.thread.h>

#include <Windows.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include <cimgui.h>
#include <cimgui_impl.h>

typedef struct valheim_QueueProcessingBeginInfo {
	valheim_VulkanContext *context;
	valheim_Queue *queue;
	valheim_Allocator *allocator;
} valheim_QueueProcessingBeginInfo;

typedef struct valheim_VulkanInitializationGraphNodeLength {
	u64 index;
	u64 length;
} valheim_VulkanInitializationGraphNodeLength;

static void *valheim_vulkanLoadAddress( const char *function, void *userData ) {
	valheim_VulkanContext *context = userData;
	return vkGetInstanceProcAddr( context->instance, function );
}

static void valheim_initVulkanInitializationGraph( valheim_VulkanInitializationGraph *graph, valheim_Allocator *allocator ) {
	valheim_initMap( &graph->nodes, sizeof( valheim_VulkanInitializationGraphNode * ), 1031, valheim_hash, allocator );
	valheim_initIndexableArray( graph->arrNodes, 1, allocator );

	const char *steps[] = {
		"init-buffer-manager",
		"init-texture-manager",
		"init-pipeline-manager",
		"init-camera",
		"init-priority-queues",
		"init-handles",
		"create-command-pool",
		"create-surface",
		"create-instance",
		"create-color-texture",
		"select-physical-device",
		"create-logical-device",
		"create-swap-chain",
		"create-scene",
		"allocate-command-buffers",
		"create-depth-texture",
		"load-pipelines",
	};

	for ( u32 iStep = 0; iStep < VALHEIM_ARRAY_LEN( steps ); ++iStep ) {
		valheim_VulkanInitializationGraphNode *node = valheim_allocate( allocator, sizeof * node );
		node->name = steps[ iStep ];

		valheim_mapInsert( &graph->nodes, node->name, valheim_stringLength( node->name ), &node, allocator );
		valheim_indexableArrayAppend( graph->arrNodes, node );
	}

	valheim_initIndexableArray( graph->sortedNodes, graph->arrNodes.length, allocator );
}

static void valheim_linkInitializationStep( valheim_VulkanInitializationGraph *graph, const char *name, const char **inDegrees, u32 inDegreeCount, valheim_Allocator *allocator ) {
	valheim_VulkanInitializationGraphNode *node;
	if ( valheim_mapFind( &graph->nodes, name, valheim_stringLength( name ), &node ) ) {
		valheim_initIndexableArray( node->inDegrees, inDegreeCount, allocator );
		for ( u32 iDeg = 0; iDeg < inDegreeCount; ++iDeg ) {
			valheim_VulkanInitializationGraphNode *dependencyNode;
			if ( valheim_mapFind( &graph->nodes, inDegrees[ iDeg ], valheim_stringLength( inDegrees[ iDeg ] ), &dependencyNode ) ) {
				if ( !dependencyNode->outDegrees.data ) {
					valheim_initIndexableArray( dependencyNode->outDegrees, 1, allocator );
				}

				valheim_indexableArrayAppend( dependencyNode->outDegrees, name );
			}

			valheim_indexableArrayAppend( node->inDegrees, inDegrees[ iDeg ] );
		}
	}
}

static void valheim_sortInitializationSteps( valheim_VulkanInitializationGraph *graph, valheim_Allocator *allocator ) {
	valheim_Queue queue;
	valheim_initQueue( sizeof( valheim_VulkanInitializationGraphNode * ), allocator, &queue );

	valheim_Map lengths;
	valheim_initMap( &lengths, sizeof( valheim_VulkanInitializationGraphNodeLength ), 1031, valheim_hash, allocator );

	for ( u32 iNode = 0; iNode < graph->arrNodes.length; ++iNode ) {
		valheim_VulkanInitializationGraphNode *node = graph->arrNodes.data[ iNode ];

		if ( node->inDegrees.length == 0 ) {
			valheim_queueEnqueue( &queue, &node, allocator );
		} else {
			valheim_VulkanInitializationGraphNodeLength length = { 0 };
			length.index = iNode;
			length.length = node->inDegrees.length;

			valheim_mapInsert( &lengths, node->name, valheim_stringLength( node->name ), &length, allocator );
		}
	}

	valheim_VulkanInitializationGraphNode *node;
	while ( valheim_queueDequeue( &queue, allocator, &node ) ) {
		valheim_indexableArrayAppend( graph->sortedNodes, node );

		for ( u32 iOut = 0; iOut < node->outDegrees.length; ++iOut ) {
			const char *name = node->outDegrees.data[ iOut ];

			valheim_VulkanInitializationGraphNodeLength length = { 0 };
			valheim_mapFind( &lengths, name, valheim_stringLength( name ), &length );

			if ( --length.length == 0 ) {
				valheim_VulkanInitializationGraphNode *node = graph->arrNodes.data[ length.index ];
				valheim_queueEnqueue( &queue, &node, allocator );
			} else {
				valheim_mapUpdate( &lengths, name, valheim_stringLength( name ), &length );
			}
		}
	}

	assert( graph->arrNodes.length == graph->sortedNodes.length && "sorted node length does not equal original node length. a dependency is not set up correctly.");

	valheim_deinitMap( &lengths, allocator );
	valheim_deinitQueue( &queue, allocator );
}

static void valheim_processKeyPress( GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods ) {
	valheim_VulkanContext *context = glfwGetWindowUserPointer( window );

	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
		context->uiInput.shouldShowUI = !context->uiInput.shouldShowUI;

		if ( context->uiInput.shouldShowUI ) {
			glfwSetInputMode( context->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		} else {
			glfwSetCursorPos( context->window, context->lastMouseX, context->lastMouseY );
			glfwSetInputMode( context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		}
	}
}

static void valheim_processInput( valheim_VulkanContext *context, f32 deltaTime ) {
	if ( glfwGetKey( context->window, GLFW_KEY_W ) == GLFW_PRESS ) {
		valheim_cameraMoveForward( context, deltaTime );
	}
	if ( glfwGetKey( context->window, GLFW_KEY_S ) == GLFW_PRESS ) {
		valheim_cameraMoveBackward( context, deltaTime );
	}
	if ( glfwGetKey( context->window, GLFW_KEY_A ) == GLFW_PRESS ) {
		valheim_cameraMoveLeft( context, deltaTime );
	}
	if ( glfwGetKey( context->window, GLFW_KEY_D ) == GLFW_PRESS ) {
		valheim_cameraMoveRight( context, deltaTime );
	}
	if ( glfwGetKey( context->window, GLFW_KEY_Q ) == GLFW_PRESS ) {
		valheim_cameraMoveDown( context, deltaTime );
	}
	if ( glfwGetKey( context->window, GLFW_KEY_E ) == GLFW_PRESS ) {
		valheim_cameraMoveUp( context, deltaTime );
	}

	if ( glfwGetKey( context->window, GLFW_KEY_P ) == GLFW_PRESS ) {
		printf( "camera position: %f, %f, %f\n", context->camera.position[ 0 ], context->camera.position[ 1 ], context->camera.position[ 2 ] );
	}
}

static void valheim_processMouseMovement( GLFWwindow *window, f64 xpos, f64 ypos ) {
	valheim_VulkanContext *context = glfwGetWindowUserPointer( window );

	const f32 fxpos = ( f32 ) xpos;
	const f32 fypos = ( f32 ) ypos;

	if ( context->firstMouseMoved ) {
		context->lastMouseX = fxpos;
		context->lastMouseY = fypos;
		context->firstMouseMoved = false;
	}

	if ( context->uiInput.shouldShowUI ) {
		return;
	}

	const f32 xoffset = fxpos - context->lastMouseX;
	const f32 yoffset = context->lastMouseY - fypos;

	context->lastMouseX = fxpos;
	context->lastMouseY = fypos;

	valheim_cameraRotate( context, xoffset, yoffset, 0.1f );
}

b8 valheim_initContext( valheim_Allocator *allocator, valheim_VulkanContext *context ) {
	valheim_zeroMemory( context, sizeof( *context ) );
	context->firstMouseMoved = true;

#ifdef VALIDATE
#undef VALIDATE
#endif

#define VALIDATE(result) if (result) {} else { valheim_deinitContext( context, allocator ); return false; }

	VALIDATE( glfwInit() == GLFW_TRUE );

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

	VALIDATE( ( context->window = glfwCreateWindow( 1280, 720, "Graph Visualizer", NULL, NULL ) ) );
	glfwSetWindowUserPointer( context->window, context );
	glfwSetCursorPosCallback( context->window, valheim_processMouseMovement );
	glfwSetCursorPos( context->window, 1280.0f / 2, 720.0f / 2 );
	glfwSetInputMode( context->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	glfwSetKeyCallback( context->window, valheim_processKeyPress );

	valheim_VulkanInitializationGraph depGraph = { 0 };
	valheim_initVulkanInitializationGraph( &depGraph, allocator );

	valheim_linkInitializationStep( &depGraph, "create-surface", ( const char *[] ) { "create-instance" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "select-physical-device", ( const char *[] ) { "create-surface" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-logical-device", ( const char *[] ) { "select-physical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-swap-chain", ( const char *[] ) { "create-logical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-command-pool", ( const char *[] ) { "create-logical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "allocate-command-buffers", ( const char *[] ) { "create-command-pool" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-depth-texture", ( const char *[] ) { "create-logical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-color-texture", ( const char *[] ) { "create-logical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "load-pipelines", ( const char *[] ) { "create-logical-device" }, 1, allocator );
	valheim_linkInitializationStep( &depGraph, "create-scene", ( const char *[] ) { "load-pipelines" }, 1, allocator );

	valheim_sortInitializationSteps( &depGraph, allocator );

	//if ( !valheim_initEngineJob( context, &depGraph, allocator ) ) {
	//	valheim_deinitContext( context, allocator );
	//	return false;
	//}
	
	VALIDATE( volkInitialize() == VK_SUCCESS );
	valheim_initBufferManager( context, allocator );
	valheim_initTextureManager( context, allocator );
	valheim_initPipelineManager( context, allocator );
	valheim_initDescriptorSetManager( context, allocator );
	valheim_initCamera( context );
	valheim_initQueue( sizeof( valheim_VulkanJob ), allocator, &context->highPriority );
	valheim_initQueue( sizeof( valheim_VulkanJob ), allocator, &context->mediumPriority );
	valheim_initQueue( sizeof( valheim_VulkanJob ), allocator, &context->lowPriority );
	valheim_initMap( &context->pipelineHandles, sizeof( valheim_VulkanPipeline ), 1024, valheim_hash, allocator );
	VALIDATE( valheim_initInstance( context ) );
	VALIDATE( glfwCreateWindowSurface( context->instance, context->window, NULL, &context->surface ) == VK_SUCCESS );
	VALIDATE( valheim_selectPhysicalDevice( context ) );
	VALIDATE( valheim_initLogicalDevice( context ) );
	VALIDATE( valheim_initSwapChain( context, allocator ) );
	VALIDATE( valheim_initSyncObjects( context, allocator ) );
	VALIDATE( valheim_initCommandPool( context ) );
	VALIDATE( valheim_initCommandBuffers( context, allocator ) );
	VALIDATE( valheim_initDepthTexture( context ) );
	VALIDATE( valheim_initColorTexture( context ) );
	VALIDATE( valheim_loadPipelines( context, allocator ) );
	VALIDATE( valheim_initVulkanScene( context, allocator, &context->worldScene ) );

	VALIDATE( valheim_loadGltfFile( context, "assets/DamagedHelmet.gltf", allocator, &context->worldScene ) );

#undef VALIDATE

	igCreateContext( NULL );
	ImGuiIO *io = igGetIO_Nil();

	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	igStyleColorsDark( NULL );

	ImGui_ImplGlfw_InitForVulkan( context->window, true );

	ImGui_ImplVulkan_InitInfo initInfo = { 0 };
	initInfo.Instance = context->instance;
	initInfo.PhysicalDevice = context->physicalDevice;
	initInfo.Device = context->device;
	initInfo.QueueFamily = context->graphicsQueueIndex;
	initInfo.Queue = context->graphicsQueue;
	initInfo.MinImageCount = context->imageCount;
	initInfo.ImageCount = context->imageCount;
	initInfo.MSAASamples = valheim_getMaxSampleCount( context );
	initInfo.DescriptorPoolSize = 1024;
	initInfo.UseDynamicRendering = true;

	ImGui_ImplVulkan_Init( &initInfo );

	ImGui_ImplVulkan_MainPipelineCreateInfo mpci = { 0 };
	mpci.MSAASamples = valheim_getMaxSampleCount( context );
	mpci.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	mpci.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	mpci.PipelineRenderingCreateInfo.pColorAttachmentFormats = &context->format.format;
	mpci.PipelineRenderingCreateInfo.depthAttachmentFormat = valheim_getDepthFormat( context );

	ImGui_ImplVulkan_CreateMainPipeline( &mpci );
	return true;
}

void valheim_deinitContext( valheim_VulkanContext *context, valheim_Allocator *allocator ) {
	if ( context->device ) {
		vkDeviceWaitIdle( context->device );
	}

	for ( u32 iImage = 0; iImage < context->imageCount; ++iImage ) {
		vkDestroySemaphore( context->device, context->imageAvailableSemaphores.data[ iImage ], NULL );
		vkDestroySemaphore( context->device, context->renderFinishedSemaphores.data[ iImage ], NULL );
		vkDestroyFence( context->device, context->inFlightFences.data[ iImage ], NULL );
		vkDestroyImageView( context->device, context->swapChainImageViews.data[ iImage ], NULL );
	}

	valheim_deinitTextureManager( context, allocator );
	valheim_deinitBufferManager( context, allocator );
	valheim_deinitDescriptorSetManager( context, allocator );
	valheim_deinitPipelineManager( context, allocator );
	valheim_deinitCommandBuffers( context, allocator );
	valheim_deinitCommandPool( context );
	valheim_deinitSwapChain( context, allocator );
	valheim_deinitLogicalDevice( context );
	valheim_deinitInstance( context );

	volkFinalize();

	glfwDestroyWindow( context->window );
	glfwTerminate();
}

void valheim_runApplication( valheim_VulkanContext *context, valheim_Allocator *allocator ) {
	f32 lastFrame = 0.0f;
	while ( !glfwWindowShouldClose( context->window ) ) {
		const f32 currentFrame = ( f32 ) glfwGetTime();
		const f32 deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		valheim_processInput( context, deltaTime );
		if ( valheim_beginScene( context, allocator ) ) {
			valheim_endScene( context );
		}
	}
}
