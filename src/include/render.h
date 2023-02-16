#ifndef RENDER_H_
#define RENDER_H_

#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include "ext/glad.h"
#include "ext/stb_image.h"
#include "file.h"

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define PROGRAM_OPENGL_INFO 1
#define PROGRAM_DEBUG_INFO 1

typedef struct vertex {
	vec2 position;
	vec3 color;
	vec2 uv;
} vertex;

typedef struct renderer {
	GLuint        VAO;
	GLuint        VBO;
	GLuint        EBO;
	const GLchar *vert_source; // FREE
	GLuint        vert_shader;
	const GLchar *frag_source; // FREE
	GLuint        frag_shader;
	GLuint        shader_program;
	vertex       *vertices;
	GLuint       *indices;
	GLint         num_vertices;
	GLint         num_indices;
	GLuint        uniform_time;
	GLuint        uniform_mouse;
	GLuint        uniform_window_size;
	GLuint        uniform_scale;
	GLuint        uniform_independent_scale;
	GLuint        uniform_offset_position;
	GLuint        uniform_tex_size;
	GLuint        uniform_lanczos;
	GLuint        uniform_lanczos_antiringing;
	GLint         texwidth, texheight, texchannels;
	GLint         texunit;
	GLuint        texture;
	stbi_uc      *texdata; // FREE
	GLfloat       value_time;
	GLint         value_lanczos;
	GLfloat       value_lanczos_antiringing;
	GLfloat       value_mouse_x, value_mouse_y;
	GLfloat       value_window_size_x, value_window_size_y;
	GLfloat       value_scale;
	GLfloat       value_ind_scale;
	GLfloat       value_offset_position_x, value_offset_position_y;
	GLfloat       value_tex_size_x, value_tex_size_y;
} renderer;

typedef struct gl_data {
	renderer background_renderer;
	renderer image_renderer;
	// renderer menu_renderer;
} gl_data;

typedef struct window {
	GLint       high_quality_resampling;
	GLfloat     lanczos_anti_ringing;
	GLint       width, height;
	GLboolean   is_glfw;
	GLFWwindow *instance;
	GLdouble    cursor_x, cursor_y;
	GLboolean   drag_mode;
	GLfloat     drag_start_x, drag_start_y;
	gl_data     data;
} window;

GLint renderer_ready(renderer *r, GLint num_vtx, GLint num_ind, vertex *vtx,
                     GLuint *ind, const GLchar *vert_path,
                     const GLchar *frag_path, const GLchar *texture_path);
GLint renderer_init(renderer *r);
void  renderer_uniform(renderer *r);
void  renderer_use(renderer *r);

// GLFW callbacks.
void glfw_error_cb(GLint e, const GLchar *desc);
void glfw_key_cb(GLFWwindow *w, GLint key, GLint scancode, GLint action,
                 GLint mods);
void glfw_framebuffer_size_cb(GLFWwindow *w, GLint fbwidth, GLint fbheight);
void glfw_cursorpos_cb(GLFWwindow *w, GLdouble x, GLdouble y);
void glfw_click_cb(GLFWwindow *w, int button, int action, int mods);
void glfw_click_cb(GLFWwindow *w, int button, int action, int mods);
void glfw_scroll_cb(GLFWwindow *w, double x, double y);

void program_log_error(const GLchar *log);
void program_log_warn(const GLchar *log);
void program_log_info(const GLchar *log);

int clean_exit(int code);

#define NUM_TRIANGLE_VERTICES 3
static vertex triangle_vertices[] = {
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
	{ { 0.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } }
};

#define NUM_TRIANGLE_INDICES 3
static GLuint triangle_indices[] = { 0, 1, 2 };

#define NUM_RECT_VERTICES 4
static vertex rect_vertices[] = {
	{ { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	{ { 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
};

#define NUM_RECT_INDICES 6
static GLuint rect_indices[] = { 0, 3, 2, 0, 1, 2 };

#endif // RENDER_H_
