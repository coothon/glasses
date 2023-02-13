#include "include/render.h"

extern window *program;

// Returns EXIT_FAILURE upon fuck-up.
GLint renderer_ready_background(window *w, GLint num_vtx, GLint num_ind, vertex *vtx,
							   GLuint *ind, const GLchar *vert_path, const GLchar *frag_path,
							   const GLchar *texture_path) {
	renderer *bg = &w->data.background_renderer;

	bg->vert_source = read_file(vert_path);
	if (!bg->vert_source) {
		program_log_error("Vertex shader for background does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	bg->frag_source = read_file(frag_path);
	if (!bg->frag_source) {
		program_log_error("Fragment shader for background does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	bg->vertices = vtx;
	bg->num_vertices = num_vtx;
	
	bg->indices = ind;
	bg->num_indices = num_ind;

	bg->texdata = stbi_load(texture_path, &bg->texwidth, &bg->texheight, &bg->texchannels, STBI_default);
	if (!bg->texdata) {
		program_log_error("Texture for background does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	bg->texunit = GL_TEXTURE0;

	return EXIT_SUCCESS;
}

// ERROR: EXIT_FAILURE.
GLint renderer_init_background(window *w) {
	renderer *bg = &w->data.background_renderer;

	glGenVertexArrays(1, &bg->VAO);
	glGenBuffers(1, &bg->VBO);
	glGenBuffers(1, &bg->EBO);
	glBindVertexArray(bg->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, bg->VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * bg->num_vertices, bg->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * bg->num_indices, bg->indices, GL_STATIC_DRAW);



	bg->vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(bg->vert_shader, 1, &bg->vert_source, NULL);
	glCompileShader(bg->vert_shader);

	/* Check. */ {
		GLint shader_status;
		GLchar shader_log[512];
		glGetShaderiv(bg->vert_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(bg->vert_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	bg->frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(bg->frag_shader, 1, &bg->frag_source, NULL);
	glCompileShader(bg->frag_shader);

	/* Check. */ {
		GLint shader_status;
		GLchar shader_log[512];
		glGetShaderiv(bg->frag_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(bg->frag_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	bg->shader_program = glCreateProgram();
	glAttachShader(bg->shader_program, bg->vert_shader);
	glAttachShader(bg->shader_program, bg->frag_shader);
	glLinkProgram(bg->shader_program);

	/* Check. */ {
		GLint program_status;
		GLchar program_log[512];
		glGetProgramiv(bg->shader_program, GL_LINK_STATUS, &program_status);
		if (!program_status) {
			glGetProgramInfoLog(bg->shader_program, 512, NULL, program_log);
			program_log_error(program_log);
			return EXIT_FAILURE;
		}
	}
	glUseProgram(bg->shader_program);

	free(bg->vert_source);
	free(bg->frag_source);
	glDeleteShader(bg->vert_shader);
	glDeleteShader(bg->frag_shader);

#if PROGRAM_OPENGL_INFO
	program_log_info("Background Shaders compiled. Program linked successfully.");
#endif

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, uv));


	glActiveTexture(bg->texunit);
	glUniform1i(glGetUniformLocation(bg->shader_program, "tex"), 0);
	glGenTextures(1, &bg->texture);
	glBindTexture(GL_TEXTURE_2D, bg->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg->texwidth, bg->texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, bg->texdata);
	stbi_image_free(bg->texdata);

	bg->uniform_time = glGetUniformLocation(bg->shader_program, "time");
	bg->uniform_scale = glGetUniformLocation(bg->shader_program, "scale");
	bg->uniform_mouse = glGetUniformLocation(bg->shader_program, "mouse");
	bg->uniform_window_size = glGetUniformLocation(bg->shader_program, "window_size");
	bg->uniform_offset_position = glGetUniformLocation(bg->shader_program, "offset_position");

	return EXIT_SUCCESS;	
}

void renderer_use(renderer *r) {
	glActiveTexture(r->texunit);
	glUseProgram(r->shader_program);
	glBindVertexArray(r->VAO);
}

void renderer_uniform_background(window *w) {
	renderer *bg = &w->data.background_renderer;

	glUniform1f(bg->uniform_time, bg->value_time);
	glUniform2f(bg->uniform_window_size, bg->value_window_size_x, bg->value_window_size_y);
	glUniform2f(bg->uniform_mouse, bg->value_mouse_x, bg->value_mouse_y);
	glUniform2f(bg->uniform_offset_position, bg->value_offset_position_x, bg->value_offset_position_y);
	glUniform2f(bg->uniform_scale, bg->value_scale_x, bg->value_offset_position_y);
}

void renderer_draw_background(window *w) {
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void glfw_error_cb(GLint e, const GLchar *desc) {
	(void)e;
	program_log_error(desc);
}

void glfw_key_cb(GLFWwindow *w, GLint key, GLint scancode, GLint action, GLint mods) {
	(void)scancode;
	(void)mods;
#if PROGRAM_DEBUG_INFO
	GLchar info_string[512] = {0};
#endif

	switch (action) {
	case GLFW_PRESS:
#if PROGRAM_DEBUG_INFO
		snprintf(info_string, (size_t)512, "Key %s pressed.", glfwGetKeyName(key, 0));
		program_log_info(info_string);
#endif
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(w, GLFW_TRUE);
			break;
		default:
			return;
		}
	default:
		return;
	}
}

void glfw_framebuffer_size_cb(GLFWwindow *w, GLint fbwidth, GLint fbheight) {
	(void)w;
#if PROGRAM_DEBUG_INFO
	GLchar info_string[512] = {0};
	snprintf(info_string, (size_t)512, "New OpenGL framebuffer size: (x: %i, y: %i).", fbwidth, fbheight);
	program_log_info(info_string);
#endif
	program->width = fbwidth;
	program->height = fbheight;
	glViewport(0, 0, fbwidth, fbheight);
}


void glfw_cursorpos_cb(GLFWwindow *w, GLdouble x, GLdouble y) {
	(void)w;
	program->cursor_x = x;
	program->cursor_y = y;
//#if PROGRAM_DEBUG_INFO
//	GLchar info_string[512] = {0};
//	snprintf(info_string, (size_t)512, "New cursor position: (x: %G, y: %G).", x, y);
//	program_log_info(info_string);
//#endif
}

void program_log_error(const GLchar *log) {
	fprintf(stderr, "[ERROR] %s\n", log);
}

void program_log_warn(const GLchar *log) {
	fprintf(stdout, "[WARN] %s\n", log);
}

void program_log_info(const GLchar *log) {
	fprintf(stdout, "[INFO] %s\n", log);
}
