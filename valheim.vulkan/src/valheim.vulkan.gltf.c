#include "valheim.vulkan.gltf.h"
#include "valheim.vulkan.scenes.h"
#include "valheim.vulkan.buffers.h"
#include "valheim.vulkan.descriptors.h"
#include "valheim.vulkan.textures.h"

#include <valheim.memory.h>
#include <valheim.strings.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <meshoptimizer.h>

typedef struct valheim_Vertex {
	vec3 position;
	vec2 uv;
	vec3 normal;
	vec3 color;
} valheim_Vertex;

static void valheim_translateVector3DToVec2(const struct aiVector3D *in, vec2 out) {
	out[0] = in->x;
	out[1] = in->y;
}

static void valheim_translateVector3DToVec3(const struct aiVector3D *in, vec3 out) {
	out[0] = in->x;
	out[1] = in->y;
	out[2] = in->z;
}

static void valheim_translateColor4DToVec3(const struct aiColor4D *in, vec3 out) {
	out[0] = in->r;
	out[1] = in->g;
	out[2] = in->b;
}

static void valheim_copyAssimpMat4ToMat4(const struct aiMatrix4x4 *in, mat4 out) {
	out[0][0] = in->a1;
	out[0][1] = in->a2;
	out[0][2] = in->a3;
	out[0][3] = in->a4;

	out[1][0] = in->b1;
	out[1][1] = in->b2;
	out[1][2] = in->b3;
	out[1][3] = in->b4;

	out[2][0] = in->c1;
	out[2][1] = in->c2;
	out[2][2] = in->c3;
	out[2][3] = in->c4;

	out[3][0] = in->d1;
	out[3][1] = in->d2;
	out[3][2] = in->d3;
	out[3][3] = in->d4;
}

static b8 valheim_traverseNode(valheim_VulkanContext *context, const struct aiScene *scene, const struct aiNode *node, valheim_VulkanScene *vulkanScene, s32 parent, s32 depth, valheim_Allocator *allocator) {
	s32 newNodeId = valheim_vulkanSceneAddNode(vulkanScene, parent, depth, allocator);

	mat4 transform;
	valheim_copyAssimpMat4ToMat4(&node->mTransformation, transform);

	glm_mat4_copy(transform, ((mat4 *) vulkanScene->localTransforms.data)[newNodeId]);

	for (u32 iMesh = 0; iMesh < node->mNumMeshes; ++iMesh) {
		s32 subNodeId = valheim_vulkanSceneAddNode(vulkanScene, newNodeId, depth + 1, allocator);

		const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[iMesh]];
		const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		valheim_IndexableArray(valheim_Vertex) vertices;
		valheim_initIndexableArray(vertices, mesh->mNumVertices, allocator);

		for (u32 i = 0; i != mesh->mNumVertices; ++i) {
			const struct aiVector3D v = mesh->mVertices[i];
			const struct aiColor4D c = mesh->mColors[0] ? mesh->mColors[0][i] : (struct aiColor4D) { 1, 1, 1, 1 };
			const struct aiVector3D t = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : (struct aiVector3D) { 0, 0, 0 };
			const struct aiVector3D normal = mesh->mNormals[i];

			valheim_Vertex vertex = {0};
			valheim_translateVector3DToVec3(&v, vertex.position);
			valheim_translateColor4DToVec3(&c, vertex.color);
			valheim_translateVector3DToVec2(&t, vertex.uv);
			valheim_translateVector3DToVec3(&normal, vertex.normal);

			valheim_indexableArrayAppend(vertices, vertex);
		}

		valheim_IndexableArray(u32) indices;
		valheim_initIndexableArray(indices, mesh->mNumFaces, allocator);

		for (u32 i = 0; i != mesh->mNumFaces; ++i) {
			for (u32 j = 0; j != 3; ++j) {
				valheim_indexableArrayAppend(indices, mesh->mFaces[i].mIndices[j]);
			}
		}

		valheim_IndexableArray(u32) remap;
		valheim_initIndexableArray(remap, indices.length, allocator);

		meshopt_generateVertexRemap(remap.data, indices.data, indices.length, vertices.data, vertices.length, sizeof(*vertices.data));
		meshopt_remapIndexBuffer(indices.data, indices.data, indices.length, remap.data);
		meshopt_remapVertexBuffer(vertices.data, vertices.data, vertices.length, sizeof(*vertices.data), remap.data);

		meshopt_optimizeVertexCache(indices.data, indices.data, indices.length, vertices.length);
		meshopt_optimizeOverdraw(indices.data, indices.data, indices.length, vertices.data, vertices.length, sizeof(valheim_Vertex), 1.05f);
		meshopt_optimizeVertexFetch(vertices.data, indices.data, indices.length, vertices.data, vertices.length, sizeof(valheim_Vertex));

		valheim_VulkanBuffer vertexBuffer;
		valheim_initVertexBuffer(context, vertices.data, sizeof(valheim_Vertex) * vertices.length, &vertexBuffer);

		valheim_VulkanBuffer indexBuffer;
		valheim_initIndexBuffer(context, indices.data, sizeof(u32) * indices.length, &indexBuffer);

		struct aiString baseColorPath;
		aiGetMaterialTexture(material, aiTextureType_BASE_COLOR, 0, &baseColorPath, NULL, NULL, NULL, NULL, NULL, NULL);

		char baseColorFullPath[256] = {0};
		valheim_formatString(baseColorFullPath, VALHEIM_ARRAY_LEN(baseColorFullPath), "%s/%s", "assets", baseColorPath.data);

		valheim_VulkanTexture baseColorTexture;
		valheim_initTexture(context, baseColorFullPath, &baseColorTexture);

		valheim_VulkanMesh vulkanMesh = {0};
		vulkanMesh.indexBuffer = indexBuffer;
		vulkanMesh.vertexBuffer = vertexBuffer;
		vulkanMesh.indexCount = (u32) indices.length;
		vulkanMesh.material.baseColorTexture = baseColorTexture;
		valheim_mapFind(&context->pipelineHandles, VALHEIM_STATIC_TEXTURED_MESH, valheim_stringLength(VALHEIM_STATIC_TEXTURED_MESH), &vulkanMesh.pipelineHandle);

		VkDescriptorImageInfo imageInfo = {0};
		imageInfo.sampler = context->textureManager.samplers.data[vulkanMesh.material.baseColorTexture];
		imageInfo.imageView = context->textureManager.imageViews.data[vulkanMesh.material.baseColorTexture];
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		valheim_DescriptorImageInfo descriptorImageInfo = {0};
		descriptorImageInfo.binding = 0;
		descriptorImageInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorImageInfo.imageInfo = &imageInfo;

		VkDescriptorType descriptorTypes[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

		VkDescriptorSetLayout descriptorSetLayouts[10] = {0};
		for (u32 iImage = 0; iImage < context->imageCount; ++iImage) {
			descriptorSetLayouts[iImage] = context->pipelineManager.destriptorSetLayouts.data[vulkanMesh.pipelineHandle];
		}

		valheim_DescriptorSetCreateInfo descriptorCreateInfo = {0};
		descriptorCreateInfo.descriptorImageInfo = &descriptorImageInfo;
		descriptorCreateInfo.descriptorImageInfoCount = 1;
		descriptorCreateInfo.descriptorTypes = descriptorTypes;
		descriptorCreateInfo.descriptorTypeCount = VALHEIM_ARRAY_LEN(descriptorTypes);
		descriptorCreateInfo.destriptorSetLayouts = descriptorSetLayouts;

		valheim_initDescriptors(context, &descriptorCreateInfo, &vulkanMesh.descriptorSetHandle, allocator);
		valheim_mapInsert(&vulkanScene->nodeMeshes, &subNodeId, sizeof(subNodeId), &vulkanMesh, allocator);

		valheim_deinitIndexableArray(vertices);
		valheim_deinitIndexableArray(indices);
		valheim_deinitIndexableArray(remap);
	}

	if (node->mMetaData) {
		for (u32 iProperty = 0; iProperty < node->mMetaData->mNumProperties; ++iProperty) {
			const struct aiMetadataEntry *entry = &node->mMetaData->mValues[iProperty];
			printf("%i\n", entry->mType);
		}
	}

	for (u32 iChild = 0; iChild < node->mNumChildren; ++iChild) {
		valheim_traverseNode(context, scene, node->mChildren[iChild], vulkanScene, newNodeId, depth + 1, allocator);
	}

	return true;
}

b8 valheim_loadGltfFile(valheim_VulkanContext *context, const char *file, valheim_Allocator *allocator, valheim_VulkanScene *vulkanScene) {
	const struct aiScene *scene = aiImportFile(file, aiProcess_Triangulate);

	if (!scene) {
		return false;
	}

	valheim_initVulkanScene(context, allocator, vulkanScene);
	valheim_traverseNode(context, scene, scene->mRootNode, vulkanScene, -1, 0, allocator);

	aiReleaseImport(scene);
	return true;
}
