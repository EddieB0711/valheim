#include "valheim.opengl.backend.h"
#include "valheim.opengl.gltf.h"

static void valheim_debugPrintMessage( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *userParam ) {
	fprintf( stderr, "%s\n", message );
	fflush( stderr );
}

static void valheim_framebufferResized( GLFWwindow *window, s32 width, s32 height ) {
	glViewport( 0, 0, width, height );
}

static void valheim_handleInput( valheim_Context *context ) {
	if ( glfwGetKey( context->window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
		glfwSetWindowShouldClose( context->window, GLFW_TRUE );
	}
}

b8 valheim_initContext( valheim_Allocator *allocator, valheim_Context *context ) {
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	context->window = glfwCreateWindow( 1280, 720, "Graph Visualizer", NULL, NULL );

	glfwMakeContextCurrent( context->window );
	glfwSetFramebufferSizeCallback( context->window, valheim_framebufferResized );

	glewExperimental = true;
	glewInit();

	glViewport( 0, 0, 1280, 720 );

	glEnable( GL_DEBUG_OUTPUT );
	glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
	glDebugMessageCallback( valheim_debugPrintMessage, NULL );

	valheim_Scene testScene;
	valheim_loadGltfScene( context, "assets/DamagedHelmet.gltf", &testScene, allocator );

	return true;
}

void valheim_deinitContext( valheim_Context *context, valheim_Allocator *allocator ) {
	glfwDestroyWindow( context->window );
	glfwTerminate();
}

void valheim_runApplication( valheim_Context *context, valheim_Allocator *allocator ) {
	while ( !glfwWindowShouldClose( context->window ) ) {
		valheim_handleInput( context );
		glClearColor( 0.4f, 0.5f, 0.8f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glfwSwapBuffers( context->window );
		glfwPollEvents();
	}
}
