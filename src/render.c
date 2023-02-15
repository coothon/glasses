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

	bg->texdata = stbi_load(texture_path, &bg->texwidth, &bg->texheight, &bg->texchannels, STBI_rgb_alpha);
	if (!bg->texdata) {
		program_log_error("Texture for background does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	bg->texunit = GL_TEXTURE0;

	return EXIT_SUCCESS;
}

GLint renderer_ready_image(window *w, GLint num_vtx, GLint num_ind, vertex *vtx,
						   GLuint *ind, const GLchar *vert_path, const GLchar *frag_path,
						   const GLchar *texture_path) {
	renderer *img = &w->data.image_renderer;

	img->vert_source = read_file(vert_path);
	if (!img->vert_source) {
		program_log_error("Vertex shader for image does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	img->frag_source = read_file(frag_path);
	if (!img->frag_source) {
		program_log_error("Fragment shader for image does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	img->vertices = vtx;
	img->num_vertices = num_vtx;
	
	img->indices = ind;
	img->num_indices = num_ind;

	img->texdata = stbi_load(texture_path, &img->texwidth, &img->texheight, &img->texchannels, STBI_rgb_alpha);
	if (!img->texdata) {
		program_log_error("Texture for image does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	img->texunit = GL_TEXTURE1;

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

	free((void *)bg->vert_source);
	bg->vert_source = NULL;
	free((void *)bg->frag_source);
	bg->frag_source = NULL;
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bg->texwidth, bg->texheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bg->texdata);
	stbi_image_free(bg->texdata);
	bg->texdata = NULL;

	bg->uniform_window_size = glGetUniformLocation(bg->shader_program, "window_size");
	bg->uniform_tex_size = glGetUniformLocation(bg->shader_program, "tex_size");

	return EXIT_SUCCESS;	
}

GLint renderer_init_image(window *w) {
	renderer *img = &w->data.image_renderer;

	glGenVertexArrays(1, &img->VAO);
	glGenBuffers(1, &img->VBO);
	glGenBuffers(1, &img->EBO);
	glBindVertexArray(img->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, img->VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * img->num_vertices, img->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, img->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * img->num_indices, img->indices, GL_STATIC_DRAW);

	img->vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(img->vert_shader, 1, &img->vert_source, NULL);
	glCompileShader(img->vert_shader);

	/* Check. */ {
		GLint shader_status;
		GLchar shader_log[512];
		glGetShaderiv(img->vert_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(img->vert_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	img->frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(img->frag_shader, 1, &img->frag_source, NULL);
	glCompileShader(img->frag_shader);

	/* Check. */ {
		GLint shader_status;
		GLchar shader_log[512];
		glGetShaderiv(img->frag_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(img->frag_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	img->shader_program = glCreateProgram();
	glAttachShader(img->shader_program, img->vert_shader);
	glAttachShader(img->shader_program, img->frag_shader);
	glLinkProgram(img->shader_program);

	/* Check. */ {
		GLint program_status;
		GLchar program_log[512];
		glGetProgramiv(img->shader_program, GL_LINK_STATUS, &program_status);
		if (!program_status) {
			glGetProgramInfoLog(img->shader_program, 512, NULL, program_log);
			program_log_error(program_log);
			return EXIT_FAILURE;
		}
	}
	glUseProgram(img->shader_program);

	free((void *)img->vert_source);
	img->vert_source = NULL;
	free((void *)img->frag_source);
	img->frag_source = NULL;
	glDeleteShader(img->vert_shader);
	glDeleteShader(img->frag_shader);

#if PROGRAM_OPENGL_INFO
	program_log_info("Image shaders compiled. Program linked successfully.");
#endif

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, uv));


	glActiveTexture(img->texunit);
	glUniform1i(glGetUniformLocation(img->shader_program, "tex"), 0);
	glGenTextures(1, &img->texture);
	glBindTexture(GL_TEXTURE_2D, img->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->texwidth, img->texheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->texdata);
	stbi_image_free(img->texdata);
	img->texdata = NULL;

	img->uniform_time = glGetUniformLocation(img->shader_program, "time");
	img->uniform_scale = glGetUniformLocation(img->shader_program, "scale");
	img->uniform_mouse = glGetUniformLocation(img->shader_program, "mouse");
	img->uniform_window_size = glGetUniformLocation(img->shader_program, "window_size");
	img->uniform_offset_position = glGetUniformLocation(img->shader_program, "offset_position");
	img->uniform_tex_size = glGetUniformLocation(img->shader_program, "tex_size");
	img->uniform_lanczos = glGetUniformLocation(img->shader_program, "is_lanczos");
	img->uniform_lanczos_antiringing = glGetUniformLocation(img->shader_program, "anti_ringing");

	return EXIT_SUCCESS;	
}

void renderer_use(renderer *r) {
	glActiveTexture(r->texunit);
	glUseProgram(r->shader_program);
	glBindVertexArray(r->VAO);
}

void renderer_uniform_background(window *w) {
	renderer *bg = &w->data.background_renderer;

	glUniform2f(bg->uniform_window_size, bg->value_window_size_x, bg->value_window_size_y);
	glUniform2f(bg->uniform_tex_size, bg->value_tex_size_x, bg->value_tex_size_y);
}

void renderer_uniform_image(window *w) {
	renderer *img = &w->data.image_renderer;

	glUniform1f(img->uniform_time, img->value_time);
	glUniform2f(img->uniform_window_size, img->value_window_size_x, img->value_window_size_y);
	glUniform2f(img->uniform_mouse, img->value_mouse_x, img->value_mouse_y);
	glUniform2f(img->uniform_offset_position, img->value_offset_position_x, img->value_offset_position_y);
	glUniform2f(img->uniform_scale, img->value_scale_x, img->value_scale_y);
	glUniform2f(img->uniform_tex_size, img->value_tex_size_x, img->value_tex_size_y);
	glUniform1i(img->uniform_lanczos, img->value_lanczos);
	glUniform1f(img->uniform_lanczos_antiringing, img->value_lanczos_antiringing);
}

void renderer_draw_background(window *w) {
	(void)w;
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void renderer_draw_image(window *w) {
	(void)w;
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int renderer_set_image(window *w, const char *new_image) {
	renderer *img = &w->data.image_renderer;

	renderer_use(img);

	int nw = 0, nh = 0, nch = 0;
	stbi_uc *new_data = stbi_load(new_image, &nw, &nh, &nch, STBI_rgb_alpha);
	if (!new_data) {
		program_log_error("Texture for image does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	img->texdata = new_data;
	img->texwidth = nw;
	img->texheight = nh;
	img->texchannels = nch;

	glActiveTexture(img->texunit);
	glUniform1i(glGetUniformLocation(img->shader_program, "tex"), 0);
	glGenTextures(1, &img->texture);
	glBindTexture(GL_TEXTURE_2D, img->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->texwidth, img->texheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->texdata);
	stbi_image_free(img->texdata);
	img->texdata = NULL;

	return EXIT_SUCCESS;
}

void glfw_error_cb(GLint e, const GLchar *desc) {
	(void)e;
	program_log_error(desc);
}

#define ANTI_RINGING_INCR 0.1f
void glfw_key_cb(GLFWwindow *w, GLint key, GLint scancode, GLint action, GLint mods) {
	(void)scancode;
	(void)mods;

	switch (action) {

	case GLFW_PRESS: {
		switch (key) {

		case GLFW_KEY_LEFT_BRACKET: {
			program->lanczos_anti_ringing = glm_clamp(program->lanczos_anti_ringing - ANTI_RINGING_INCR, 0.0f, 1.0f);
			printf("Anti-ringing: %f\n", program->lanczos_anti_ringing);
		} break;

		case GLFW_KEY_RIGHT_BRACKET: {
			program->lanczos_anti_ringing = glm_clamp(program->lanczos_anti_ringing + ANTI_RINGING_INCR, 0.0f, 1.0f);
		} break;

		case GLFW_KEY_H: {
			program->high_quality_resampling = !program->high_quality_resampling;
		} break;

		case GLFW_KEY_L: {
			glActiveTexture(0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} break;

		case GLFW_KEY_N: {
			glActiveTexture(0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} break;

		case GLFW_KEY_ESCAPE: {
			glfwSetWindowShouldClose(w, GLFW_TRUE);
		} break;

		default: return;
		}
	} break;

	default: return;
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

int clean_exit(window *w, int code) {
	if (!w) return code;
	
	// Clean up background.
	if (w->data.background_renderer.vert_source) free((void *)w->data.background_renderer.vert_source);
	if (w->data.background_renderer.frag_source) free((void *)w->data.background_renderer.frag_source);
	if (w->data.background_renderer.texdata) stbi_image_free(w->data.background_renderer.texdata);

	// Clean up image.
	if (w->data.image_renderer.vert_source) free((void *)w->data.image_renderer.vert_source);
	if (w->data.image_renderer.frag_source) free((void *)w->data.image_renderer.frag_source);
	if (w->data.image_renderer.texdata) stbi_image_free(w->data.image_renderer.texdata);

	if (w->is_glfw) {
		if (w->instance) glfwDestroyWindow(w->instance);
		glfwTerminate();
	}

	free((void *)w);
	return code;
}
