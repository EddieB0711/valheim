#include "valheim.opengl.gltf.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer.h>

typedef struct valheim_Vertex {
	vec3 position;
	vec3 texcoord;
	vec4 color;
	vec3 normal;
} valheim_Vertex;

static void valheim_translateVector3DToVec3( const struct aiVector3D *in, vec3 out ) {
	out[ 0 ] = in->x;
	out[ 1 ] = in->y;
	out[ 2 ] = in->z;
}

static void valheim_translateColor4DToVec4( const struct aiColor4D *in, vec4 out ) {
	out[ 0 ] = in->r;
	out[ 1 ] = in->g;
	out[ 2 ] = in->b;
	out[ 3 ] = in->a;
}

static b8 valheim_traverseNode( valheim_Context *context, const struct aiScene *aiScene, const struct aiNode *aiNode, valheim_Scene *scene, valheim_Allocator *allocator ) {
	for ( u32 iMesh = 0; iMesh < aiNode->mNumMeshes; ++iMesh ) {
		const struct aiMesh *mesh = aiScene->mMeshes[ aiNode->mMeshes[ iMesh ] ];

		valheim_Array( valheim_Vertex ) vertices;
		valheim_initArray( vertices, mesh->mNumVertices, allocator );

		for ( u32 iVertex = 0; iVertex < mesh->mNumVertices; ++iVertex ) {
			const struct aiVector3D pos = mesh->mVertices[ iVertex ];
			const struct aiVector3D texcoord = mesh->mTextureCoords[ 0 ] ? mesh->mTextureCoords[ 0 ][ iVertex ] : ( struct aiVector3D ) { 0, 0, 0 };
			const struct aiColor4D color = mesh->mColors[ 0 ] ? mesh->mColors[ 0 ][ iVertex ] : ( struct aiColor4D ) { 1.0f, 1.0f, 1.0f };
			const struct aiVector3D normal = mesh->mNormals[ iVertex ];

			valheim_Vertex vertex = { 0 };
			valheim_translateVector3DToVec3( &pos, vertex.position );
			valheim_translateVector3DToVec3( &texcoord, vertex.texcoord );
			valheim_translateColor4DToVec4( &color, vertex.color );
			valheim_translateVector3DToVec3( &normal, vertex.normal );

			valheim_arrayAppend( vertices, vertex );
		}

		valheim_Array( u32 ) indices;
		valheim_initArray( indices, mesh->mNumFaces, allocator );

		for ( u32 iFace = 0; iFace < mesh->mNumFaces; ++iFace ) {
			for ( u32 iIndex = 0; iIndex < 3; ++iIndex ) {
				valheim_arrayAppend( indices, mesh->mFaces[ iFace ].mIndices[ iIndex ] );
			}
		}

		valheim_Array( u32 ) remap;
		valheim_initArray( remap, indices.length, allocator );

		meshopt_generateVertexRemap( remap.data, indices.data, indices.length, vertices.data, vertices.length, sizeof( *vertices.data ) );
		meshopt_remapIndexBuffer( indices.data, indices.data, indices.length, remap.data );
		meshopt_remapVertexBuffer( vertices.data, vertices.data, vertices.length, sizeof( *vertices.data ), remap.data );

		meshopt_optimizeVertexCache( indices.data, indices.data, indices.length, vertices.length );

		valheim_Mesh sceneMesh = { 0 };

		glCreateBuffers( 1, &sceneMesh.vbo );
		glNamedBufferStorage( sceneMesh.vbo, sizeof( valheim_Vertex ) * vertices.length, vertices.data, GL_DYNAMIC_STORAGE_BIT );

		glCreateBuffers( 1, &sceneMesh.ibo );
		glNamedBufferStorage( sceneMesh.ibo, sizeof( u32 ) * indices.length, indices.data, GL_DYNAMIC_STORAGE_BIT );

		glCreateVertexArrays( 1, &sceneMesh.vao );

		glVertexArrayVertexBuffer( sceneMesh.vao, 0, sceneMesh.vbo, 0, sizeof( valheim_Vertex ) );
		glVertexArrayElementBuffer( sceneMesh.vao, sceneMesh.ibo );

		glEnableVertexArrayAttrib( sceneMesh.vao, 0 );
		glEnableVertexArrayAttrib( sceneMesh.vao, 1 );
		glEnableVertexArrayAttrib( sceneMesh.vao, 2 );
		glEnableVertexArrayAttrib( sceneMesh.vao, 3 );

		glVertexArrayAttribFormat( sceneMesh.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof( valheim_Vertex, position ) );
		glVertexArrayAttribFormat( sceneMesh.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof( valheim_Vertex, texcoord ) );
		glVertexArrayAttribFormat( sceneMesh.vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof( valheim_Vertex, color ) );
		glVertexArrayAttribFormat( sceneMesh.vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof( valheim_Vertex, normal ) );

		glVertexArrayAttribBinding( sceneMesh.vao, 0, 0 );
		glVertexArrayAttribBinding( sceneMesh.vao, 1, 0 );
		glVertexArrayAttribBinding( sceneMesh.vao, 2, 0 );
		glVertexArrayAttribBinding( sceneMesh.vao, 3, 0 );
	}
}

b8 valheim_loadGltfScene( valheim_Context *context, const char *file, valheim_Scene *scene, valheim_Allocator *allocator ) {
	const struct aiScene *aiScene = aiImportFile( file, aiProcess_Triangulate );
	if ( !aiScene ) {
		return false;
	}

	valheim_traverseNode( context, aiScene, aiScene->mRootNode, scene, allocator );
	return true;
}
