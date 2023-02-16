// #define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
// #include <unistd.h>

// #define DESIRED_FPS (long)(256)
//  Nanoseconds! Fuck! And here I was expecting milliseconds.
// #define DESIRED_FRAMETIME (long)(1000000000) / DESIRED_FPS

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
	stbi_set_flip_vertically_on_load(true);

	program = (window *)calloc(1, sizeof(window));
	if (!program) {
		program_log_error("Memory allocation failed. Exiting. . . .");
		clean_exit(EXIT_FAILURE);
	}

	// Defaults.
	program->lanczos_anti_ringing    = 0.8f;
	program->high_quality_resampling = GL_TRUE;
	program->drag_mode               = GL_FALSE;

	// const struct timespec frametime = (const struct timespec){
	//	.tv_sec  = 0,
	//	.tv_nsec = DESIRED_FRAMETIME,
	// };

	// GLFW init.
	if (!(program->is_glfw = glfwInit())) {
		program_log_error("GLFW initialization failed. Exiting. . . .");
		clean_exit(EXIT_FAILURE);
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
		clean_exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(program->instance);

	// GLAD init.
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwGetFramebufferSize(program->instance, &program->width,
	                       &program->height);
	glViewport(0, 0, program->width, program->height);
	glfwSetFramebufferSizeCallback(program->instance, glfw_framebuffer_size_cb);
	glfwSetKeyCallback(program->instance, glfw_key_cb);
	glfwSetCursorPosCallback(program->instance, glfw_cursorpos_cb);
	glfwSetMouseButtonCallback(program->instance, glfw_click_cb);
	glfwSetScrollCallback(program->instance, glfw_scroll_cb);

	glfwSwapInterval(0);
	glClearColor(0.7f, 0.1f, 0.8f, 1.0f);

	GLint result = renderer_ready(
		&program->data.background_renderer, NUM_RECT_VERTICES, NUM_RECT_INDICES,
		rect_vertices, rect_indices, "./background.vert", "background.frag",
		"background2.png");
	if (result == EXIT_FAILURE) clean_exit(EXIT_FAILURE);
	result = renderer_init(&program->data.background_renderer);
	if (result == EXIT_FAILURE) clean_exit(EXIT_FAILURE);

	result = renderer_ready(&program->data.image_renderer, NUM_RECT_VERTICES,
	                        NUM_RECT_INDICES, rect_vertices, rect_indices,
	                        "./image.vert", "image.frag", "background1.png");
	if (result == EXIT_FAILURE) clean_exit(EXIT_FAILURE);
	result = renderer_init(&program->data.image_renderer);
	if (result == EXIT_FAILURE) clean_exit(EXIT_FAILURE);

	// Wireframe mode:
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glfwSwapInterval(0);

	// Main event loop. This is where everything happens.
	renderer *bg  = &program->data.background_renderer;
	renderer *img = &program->data.image_renderer;
	do {
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw background.
		renderer_use(bg);
		bg->value_window_size_x     = (GLfloat)program->width;
		bg->value_window_size_y     = (GLfloat)program->height;
		bg->value_tex_size_x        = (GLfloat)bg->texwidth;
		bg->value_tex_size_y        = (GLfloat)bg->texheight;
		bg->value_scale             = 1.0f;
		bg->value_offset_position_x = 0.0f;
		bg->value_offset_position_y = 0.0f;
		renderer_uniform(bg);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Draw image.
		renderer_use(img);
		img->value_time                = (GLfloat)glfwGetTime();
		img->value_lanczos             = program->high_quality_resampling;
		img->value_lanczos_antiringing = program->lanczos_anti_ringing;
		img->value_window_size_x       = (GLfloat)program->width;
		img->value_window_size_y       = (GLfloat)program->height;
		img->value_tex_size_x          = (GLfloat)img->texwidth;
		img->value_tex_size_y          = (GLfloat)img->texheight;
		if (program->width < program->height) {
			img->value_ind_scale =
				(img->value_tex_size_x > img->value_tex_size_y)
					? program->width / img->value_tex_size_x
					: program->width / img->value_tex_size_y;
		} else {
			img->value_ind_scale =
				(img->value_tex_size_x > img->value_tex_size_y)
					? program->height / img->value_tex_size_x
					: program->height / img->value_tex_size_y;
		}
		renderer_uniform(img);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Prepare your eyes. . . .
		glfwSwapBuffers(program->instance);
		// nanosleep(&frametime, NULL);
		glfwWaitEvents();
	} while (!glfwWindowShouldClose(program->instance));

	return clean_exit(EXIT_SUCCESS);
}
