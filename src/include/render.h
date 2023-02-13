#ifndef RENDER_H_
#define RENDER_H_

#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include "ext/glad.h"
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "ext/stb_image.h"

#include "file.h"

#define PROGRAM_OPENGL_INFO 1
#define PROGRAM_DEBUG_INFO 1

typedef struct vertex {
	vec2 position;
	vec3 color;
	vec2 uv;
} __attribute__((packed)) vertex;

typedef struct renderer {
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	const GLchar *vert_source; // FREE
	GLuint vert_shader;
	const GLchar *frag_source; // FREE
	GLuint frag_shader;
	GLuint shader_program;
	vertex *vertices; // FREE
	GLuint *indices; // FREE
	GLint num_vertices;
	GLint num_indices;
	GLuint uniform_time;
	GLuint uniform_mouse;
	GLuint uniform_window_size;
	GLuint uniform_scale;
	GLuint uniform_offset_position;
	GLint texwidth, texheight, texchannels;
	GLint texunit;
	GLuint texture;
	stbi_uc *texdata;
	GLfloat value_time;
	GLfloat value_mouse_x, value_mouse_y;
	GLfloat value_window_size_x, value_window_size_y;
	GLfloat value_scale_x, value_scale_y;
	GLfloat value_offset_position_x, value_offset_position_y;
} __attribute__((packed)) renderer;

typedef struct gl_data {
	renderer background_renderer;
	//renderer image_renderer;
	//renderer menu_renderer;
} gl_data;

typedef struct window {
	GLint width, height;
	GLboolean is_glfw;
	GLFWwindow *instance;
	GLdouble cursor_x, cursor_y;
	gl_data data;
} window;

GLint renderer_ready_background(window *w, GLint num_vtx, GLint num_ind, vertex *vtx, GLuint *ind, const GLchar *vert_path, const GLchar *frag_path, const GLchar *texture_path);
GLint renderer_init_background(window *w);
void renderer_uniform_background(window *w);
void renderer_draw_background(window *w);
void renderer_use(renderer *r);

// GLFW callbacks.
void glfw_error_cb(GLint e, const GLchar *desc);
void glfw_key_cb(GLFWwindow *w, GLint key, GLint scancode, GLint action, GLint mods);
void glfw_framebuffer_size_cb(GLFWwindow *w, GLint fbwidth, GLint fbheight);
void glfw_cursorpos_cb(GLFWwindow *w, GLdouble x, GLdouble y);

void program_log_error(const GLchar *log);
void program_log_warn(const GLchar *log);
void program_log_info(const GLchar *log);

#define NUM_TRIANGLE_VERTICES 3
static vertex triangle_vertices[] = {
	{
		{-0.5f, -0.5f},
		{1.0f, 0.0f, 0.0f},
		{-1.0f, -1.0f}},
	{
		{0.5f, -0.5f},
		{0.0f, 1.0f, 0.0f},
		{1.0f, -1.0f}},
	{
		{0.0f, 0.5f},
		{0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f}}
};

#define NUM_TRIANGLE_INDICES 3
static GLuint triangle_indices[] = {
	0, 1, 2
};

#define NUM_RECT_VERTICES 4
static vertex rect_vertices[] = {{
		{1.0f,  1.0f},
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f}
	}, {
		{1.0f,  -1.0f},
		{1.0f, 1.0f, 1.0f},
		{1.0f, 0.0f}
	}, {
		{-1.0f,  -1.0f},
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f}
	}, {
		{-1.0f,  1.0f},
		{1.0f, 1.0f, 1.0f},
		{0.0f, 1.0f}
	}
};

#define NUM_RECT_INDICES 6
static GLuint rect_indices[] = {
	0, 3, 2,
	0, 1, 2
};


#endif // RENDER_H_
