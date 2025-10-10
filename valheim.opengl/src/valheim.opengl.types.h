#pragma once

#include <valheim.defines.h>
#include <valheim.array.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

typedef s32 valheim_Handle;
typedef valheim_Handle valheim_TextureHandle;

typedef struct valheim_Mesh {
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
} valheim_Mesh;

typedef struct valheim_Material {
	valheim_TextureHandle albedo;
} valheim_Material;

typedef struct valheim_SceneHierarchy {
	s32 parent;
	s32 level;
} valheim_SceneHierarchy;

typedef struct valheim_Scene {
	valheim_Array( valheim_SceneHierarchy ) hierarchies;
	valheim_Array( valheim_Mesh ) nodeMeshes;
	valheim_Array( valheim_Material ) nodeMaterials;
	valheim_Array( mat4 ) localTransforms;
} valheim_Scene;

typedef struct valheim_Context {
	GLFWwindow *window;
} valheim_Context;