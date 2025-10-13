#include "valheim.vulkan.scenes.h"

#include <valheim.hashing.h>

static u64 valheim_integerHash( const void *key, u64 keyStride, u64 seed ) {
	s32 *k = key;
	return *k;
}

static void valheim_recalculateHeirarchialTransforms( valheim_VulkanScene *scene, valheim_VulkanSceneHeirarchy *heirarchy, s32 node ) {
	if ( heirarchy->parent == -1 ) {
		glm_mat4_copy( ( ( mat4 * ) scene->localTransforms.data )[ node ], ( ( mat4 * ) scene->globalTransforms.data )[ node ] );
	} else {
		glm_mat4_mul( ( ( mat4 * ) scene->globalTransforms.data )[ heirarchy->parent ], ( ( mat4 * ) scene->localTransforms.data )[ node ], ( ( mat4 * ) scene->globalTransforms.data )[ node ] );
	}

	for ( s32 iChild = heirarchy->firstChild; iChild != -1; iChild = scene->heirarchies.data[ iChild ].nextSibling ) {
		valheim_recalculateHeirarchialTransforms( scene, &scene->heirarchies.data[ iChild ], iChild );
	}

	for ( s32 iSibling = heirarchy->nextSibling; iSibling != -1; iSibling = scene->heirarchies.data[ iSibling ].nextSibling ) {
		valheim_recalculateHeirarchialTransforms( scene, &scene->heirarchies.data[ iSibling ], iSibling );
	}
}

static void valheim_renderSceneHierarchy( valheim_VulkanContext *context, valheim_VulkanScene *scene, valheim_VulkanSceneHeirarchy *heirarchy, s32 node, VkCommandBuffer commandBuffer, valheim_VulkanFrameData *frameData ) {
	valheim_VulkanMesh mesh = { 0 };
	if ( valheim_mapFind( &scene->nodeMeshes, &node, sizeof( node ), &mesh ) ) {
		glm_mat4_copy( ( ( mat4 * ) scene->globalTransforms.data )[ node ], frameData->model );

		const VkDescriptorSet *descriptorSets = context->descriptorSetManager.descriptorSets.data[ mesh.descriptorSetHandle ];

		vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineManager.pipelines.data[ mesh.pipelineHandle ] );
		vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineManager.pipelineLayouts.data[ mesh.pipelineHandle ], 0, 1, &descriptorSets[ context->currentFrame ], 0, NULL );

		vkCmdPushConstants( commandBuffer, context->pipelineManager.pipelineLayouts.data[ mesh.pipelineHandle ], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( *frameData ), frameData );

		const VkDeviceSize offsets[ 1 ] = { 0 };
		vkCmdBindVertexBuffers( commandBuffer, 0, 1, &context->bufferManager.buffers.data[ mesh.vertexBuffer ], offsets );

		vkCmdBindIndexBuffer( commandBuffer, context->bufferManager.buffers.data[ mesh.indexBuffer ], 0, VK_INDEX_TYPE_UINT32 );
		vkCmdDrawIndexed( commandBuffer, mesh.indexCount, 1, 0, 0, 0 );
	}

	for ( s32 iChild = heirarchy->firstChild; iChild != -1; iChild = scene->heirarchies.data[ iChild ].nextSibling ) {
		valheim_renderSceneHierarchy( context, scene, &scene->heirarchies.data[ iChild ], iChild, commandBuffer, frameData );
	}

	for ( s32 iSibling = heirarchy->nextSibling; iSibling != -1; iSibling = scene->heirarchies.data[ iSibling ].nextSibling ) {
		valheim_renderSceneHierarchy( context, scene, &scene->heirarchies.data[ iSibling ], iSibling, commandBuffer, frameData );
	}
}

b8 valheim_initVulkanScene( valheim_VulkanContext *context, valheim_Allocator *allocator, valheim_VulkanScene *scene ) {
	valheim_initIndexableArray( scene->heirarchies, 1, allocator );
	valheim_initMap( &scene->nodeMeshes, sizeof( valheim_VulkanMesh ), 1031, valheim_integerHash, allocator );
	valheim_initMap( &scene->nodeMaterials, sizeof( valheim_VulkanMaterial ), 1031, valheim_integerHash, allocator );
	valheim_initArray( sizeof( mat4 ), 1, allocator, &scene->localTransforms );
	valheim_initArray( sizeof( mat4 ), 1, allocator, &scene->globalTransforms );
	return true;
}

s32 valheim_vulkanSceneAddNode( valheim_VulkanScene *scene, s32 parent, s32 depth, valheim_Allocator *allocator ) {
	s32 newNodeId = ( s32 ) scene->heirarchies.length;

	valheim_VulkanSceneHeirarchy heirarchy = { 0 };
	heirarchy.parent = parent;
	heirarchy.firstChild = -1;
	heirarchy.nextSibling = -1;
	heirarchy.lastSibling = -1;
	heirarchy.depth = depth;

	valheim_indexableArrayAppend( scene->heirarchies, heirarchy );

	mat4 identity;
	glm_mat4_identity( identity );

	valheim_arrayAppend( &scene->globalTransforms, identity, allocator );
	valheim_arrayAppend( &scene->localTransforms, identity, allocator );

	if ( parent > -1 ) {
		s32 firstChild = scene->heirarchies.data[ parent ].firstChild;
		if ( firstChild == -1 ) {
			scene->heirarchies.data[ parent ].firstChild = newNodeId;
			scene->heirarchies.data[ newNodeId ].lastSibling = newNodeId;
		} else {
			s32 lastSibling = scene->heirarchies.data[ firstChild ].lastSibling;

			scene->heirarchies.data[ lastSibling ].nextSibling = newNodeId;
			scene->heirarchies.data[ firstChild ].lastSibling = newNodeId;
		}
	}

	return newNodeId;
}

void valheim_vulkanSceneRecalculateTransforms( valheim_VulkanScene *scene ) {
	valheim_recalculateHeirarchialTransforms( scene, &scene->heirarchies.data[ 0 ], 0 );
}

void valheim_vulkanSceneRender( valheim_VulkanContext *context, valheim_VulkanScene *scene, VkCommandBuffer commandBuffer, valheim_VulkanFrameData *frameData ) {
	valheim_renderSceneHierarchy( context, scene, &scene->heirarchies.data[ 0 ], 0, commandBuffer, frameData );
}
