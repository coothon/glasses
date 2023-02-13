#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include "include/ext/glad.h"
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include "include/ext/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "include/file.h"
#include "include/render.h"

window *program = NULL;

int main(void) {
	program = (window *)calloc(1, sizeof(window));

	if (!program) {
		program_log_error("Memory allocation failed. Exiting. . . .");
		return EXIT_FAILURE;
	}

	// GLFW init.
	if (!(program->is_glfw = glfwInit())) {
		program_log_error("GLFW initialization failed. Exiting. . . .");
		return EXIT_FAILURE;
	}

	glfwSetErrorCallback(glfw_error_cb);
	
	// Window creation config.
	glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	program->instance = glfwCreateWindow(1280, 720, "Glasses", NULL, NULL);
	if (!program->instance) {
		program_log_error("GLFW window Creation failed. Exiting. . . .");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(program->instance);

	// GLAD init.
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwGetFramebufferSize(program->instance, &program->width, &program->height);
	glViewport(0, 0, program->width, program->height);
	glfwSetFramebufferSizeCallback(program->instance, glfw_framebuffer_size_cb);
	glfwSetKeyCallback(program->instance, glfw_key_cb);
	glfwSetCursorPosCallback(program->instance, glfw_cursorpos_cb);

	glfwSwapInterval(0);
	glClearColor(0.7f, 0.1f, 0.8f, 1.0f);

	GLint result = renderer_ready_background(program, NUM_RECT_VERTICES, NUM_RECT_INDICES,
											 rect_vertices, rect_indices, "./clip_space.vert",
											 "./frag.glsl", "./background.png");
	if (result == EXIT_FAILURE) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	result = renderer_init_background(program);
	if (result == EXIT_FAILURE) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Wireframe mode:
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glfwSwapInterval(1);

	// Main event loop. This is where everything happens.
	renderer *bg = &program->data.background_renderer;
	while (!glfwWindowShouldClose(program->instance)) {
		glClear(GL_COLOR_BUFFER_BIT);

		// Update background uniforms.
		renderer_use(bg);
		bg->value_time = (GLfloat)glfwGetTime();
		bg->value_mouse_x = (GLfloat)program->cursor_x;
		bg->value_mouse_y = (GLfloat)program->cursor_y;
		bg->value_window_size_x = (GLfloat)program->width;
		bg->value_window_size_y = (GLfloat)program->height;
		bg->value_scale_x = 1.0f;
		bg->value_scale_y = 1.0f;
		bg->value_offset_position_x = 0.0f;
		bg->value_offset_position_y = 0.0f;
		renderer_uniform_background(program);
		renderer_draw_background(program);

		// Prepare your eyes. . . .
		glfwSwapBuffers(program->instance);
		glfwPollEvents();
	}

    return EXIT_SUCCESS;
}
