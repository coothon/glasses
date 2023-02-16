#include "include/render.h"

extern window *program;

// Returns EXIT_FAILURE upon fuck-up.
GLint renderer_ready(renderer *r, GLint num_vtx, GLint num_ind, vertex *vtx,
                     GLuint *ind, const GLchar *vert_path,
                     const GLchar *frag_path, const GLchar *texture_path) {
	// Ensure unique texture bindings.
	const int units[3] = {
		GL_TEXTURE0,
		GL_TEXTURE1,
		GL_TEXTURE2,
	};
	static int unit = 0;

	r->vert_source = read_file(vert_path);
	if (!r->vert_source) {
		program_log_error("Vertex shader does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	r->frag_source = read_file(frag_path);
	if (!r->frag_source) {
		program_log_error("Fragment shader does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	r->vertices     = vtx;
	r->num_vertices = num_vtx;

	r->indices     = ind;
	r->num_indices = num_ind;

	r->texdata = stbi_load(texture_path, &r->texwidth, &r->texheight,
	                       &r->texchannels, STBI_rgb_alpha);
	if (!r->texdata) {
		program_log_error("Texture does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	r->texunit = units[unit++];

	return EXIT_SUCCESS;
}

// ERROR: EXIT_FAILURE.
GLint renderer_init(renderer *r) {
	// Unique texture locations per call; must be hardcoded into the
	// shader of choice. The first call will get a location of 0,
	// the next 1, and so on.
	static int tex_loc = 0;

	r->value_scale = 1.0f;
	r->value_ind_scale = 1.0f;

	glGenVertexArrays(1, &r->VAO);
	glGenBuffers(1, &r->VBO);
	glGenBuffers(1, &r->EBO);
	glBindVertexArray(r->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, r->VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * r->num_vertices, r->vertices,
	             GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * r->num_indices,
	             r->indices, GL_STATIC_DRAW);

	r->vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(r->vert_shader, 1, &r->vert_source, NULL);
	glCompileShader(r->vert_shader);

	/* Check. */ {
		GLint  shader_status;
		GLchar shader_log[512];
		glGetShaderiv(r->vert_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(r->vert_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	r->frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(r->frag_shader, 1, &r->frag_source, NULL);
	glCompileShader(r->frag_shader);

	/* Check. */ {
		GLint  shader_status;
		GLchar shader_log[512];
		glGetShaderiv(r->frag_shader, GL_COMPILE_STATUS, &shader_status);
		if (!shader_status) {
			glGetShaderInfoLog(r->frag_shader, 512, NULL, shader_log);
			program_log_error(shader_log);
			return EXIT_FAILURE;
		}
	}

	r->shader_program = glCreateProgram();
	glAttachShader(r->shader_program, r->vert_shader);
	glAttachShader(r->shader_program, r->frag_shader);
	glLinkProgram(r->shader_program);

	/* Check. */ {
		GLint  program_status;
		GLchar program_log[512];
		glGetProgramiv(r->shader_program, GL_LINK_STATUS, &program_status);
		if (!program_status) {
			glGetProgramInfoLog(r->shader_program, 512, NULL, program_log);
			program_log_error(program_log);
			return EXIT_FAILURE;
		}
	}
	glUseProgram(r->shader_program);

	glDeleteShader(r->vert_shader);
	glDeleteShader(r->frag_shader);

#if PROGRAM_OPENGL_INFO
	program_log_info("Image shaders compiled. Program linked successfully.");
#endif

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
	                      (void *)offsetof(vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
	                      (void *)offsetof(vertex, uv));


	glActiveTexture(r->texunit);
	glUniform1i(glGetUniformLocation(r->shader_program, "tex"), tex_loc++);
	glGenTextures(1, &r->texture);
	glBindTexture(GL_TEXTURE_2D, r->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, r->texwidth, r->texheight, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, r->texdata);

	r->uniform_time  = glGetUniformLocation(r->shader_program, "time");
	r->uniform_scale = glGetUniformLocation(r->shader_program, "scale");
	r->uniform_independent_scale = glGetUniformLocation(r->shader_program, "ind_scale");
	r->uniform_mouse = glGetUniformLocation(r->shader_program, "mouse");
	r->uniform_window_size =
		glGetUniformLocation(r->shader_program, "window_size");
	r->uniform_offset_position =
		glGetUniformLocation(r->shader_program, "offset_position");
	r->uniform_tex_size = glGetUniformLocation(r->shader_program, "tex_size");
	r->uniform_lanczos  = glGetUniformLocation(r->shader_program, "is_lanczos");
	r->uniform_lanczos_antiringing =
		glGetUniformLocation(r->shader_program, "anti_ringing");

	return EXIT_SUCCESS;
}

void renderer_use(renderer *r) {
	glBindVertexArray(r->VAO);
	glUseProgram(r->shader_program);
	glActiveTexture(r->texunit);
}

void renderer_uniform(renderer *r) {
	glUniform1f(r->uniform_time, r->value_time);
	glUniform2f(r->uniform_window_size, r->value_window_size_x,
	            r->value_window_size_y);
	glUniform2f(r->uniform_mouse, r->value_mouse_x, r->value_mouse_y);
	glUniform2f(r->uniform_offset_position, r->value_offset_position_x,
	            r->value_offset_position_y);
	glUniform1f(r->uniform_scale, r->value_scale);
	glUniform1f(r->uniform_independent_scale, r->value_ind_scale);
	glUniform2f(r->uniform_tex_size, r->value_tex_size_x, r->value_tex_size_y);
	glUniform1i(r->uniform_lanczos, r->value_lanczos);
	glUniform1f(r->uniform_lanczos_antiringing, r->value_lanczos_antiringing);
}

int renderer_set_texture(renderer *r, const char *new_image) {

	if (r->texdata) {
		stbi_image_free(r->texdata);
		r->texdata = NULL;
	}

	int      nw = 0, nh = 0, nch = 0;
	stbi_uc *new_data = stbi_load(new_image, &nw, &nh, &nch, STBI_rgb_alpha);
	if (!new_data) {
		program_log_error("Texture for image does not exist in filesystem!");
		return EXIT_FAILURE;
	}

	r->texdata     = new_data;
	r->texwidth    = nw;
	r->texheight   = nh;
	r->texchannels = nch;

	renderer_use(r);
	glUniform1i(glGetUniformLocation(r->shader_program, "tex"), 0);
	glGenTextures(1, &r->texture);
	glBindTexture(GL_TEXTURE_2D, r->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, r->texwidth, r->texheight, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, r->texdata);

	return EXIT_SUCCESS;
}

void glfw_error_cb(GLint e, const GLchar *desc) {
	(void)e;
	program_log_error(desc);
}

#define ANTI_RINGING_INCR 0.1f
void glfw_key_cb(GLFWwindow *w, GLint key, GLint scancode, GLint action,
                 GLint mods) {
	(void)scancode;
	(void)mods;

	switch (action) {
	case GLFW_PRESS: {
		switch (key) {
		case GLFW_KEY_LEFT_BRACKET: {
			program->lanczos_anti_ringing = glm_clamp(
				program->lanczos_anti_ringing - ANTI_RINGING_INCR, 0.0f, 1.0f);
		} break;

		case GLFW_KEY_RIGHT_BRACKET: {
			program->lanczos_anti_ringing = glm_clamp(
				program->lanczos_anti_ringing + ANTI_RINGING_INCR, 0.0f, 1.0f);
		} break;

		case GLFW_KEY_H: {
			program->high_quality_resampling =
				!program->high_quality_resampling;
		} break;

		case GLFW_KEY_L: {
			glActiveTexture(GL_TEXTURE1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} break;

		case GLFW_KEY_N: {
			glActiveTexture(GL_TEXTURE1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} break;

		case GLFW_KEY_ESCAPE: {
			glfwSetWindowShouldClose(w, GLFW_TRUE);
		} break;

		default:
			return;
		}
	} break;

	default:
		return;
	}
}

void glfw_framebuffer_size_cb(GLFWwindow *w, GLint fbwidth, GLint fbheight) {
	(void)w;
#if PROGRAM_DEBUG_INFO
	GLchar info_string[512] = { 0 };
	snprintf(info_string, (size_t)512,
	         "New OpenGL framebuffer size: (x: %i, y: %i).", fbwidth, fbheight);
	program_log_info(info_string);
#endif
	program->width  = fbwidth;
	program->height = fbheight;
	glViewport(0, 0, fbwidth, fbheight);
	
	program->drag_mode = GL_FALSE;
	program->data.image_renderer.value_offset_position_x = 0.0f;
	program->data.image_renderer.value_offset_position_y = 0.0f;
	program->data.image_renderer.value_scale = 1.0f;
}


#define GLFW_TO_SCREEN_X(pos) (GLfloat)((pos)) - ((GLfloat)(program->width) / 2.0f)
#define GLFW_TO_SCREEN_Y(pos) -((GLfloat)((pos)) - ((GLfloat)(program->height) / 2.0f))

void glfw_cursorpos_cb(GLFWwindow *w, GLdouble x, GLdouble y) {
	(void)w;
	program->cursor_x = x;
	program->cursor_y = y;

	if (program->drag_mode) {
		program->data.image_renderer.value_offset_position_x = GLFW_TO_SCREEN_X(x) - program->drag_start_x;
		program->data.image_renderer.value_offset_position_y = GLFW_TO_SCREEN_Y(y) - program->drag_start_y;
	}
	// #if PROGRAM_DEBUG_INFO
	//	GLchar info_string[512] = {0};
	//	snprintf(info_string, (size_t)512, "New cursor position: (x: %G, y:
	//%G).", x, y); 	program_log_info(info_string); #endif
}

void glfw_click_cb(GLFWwindow *w, int button, int action, int mods) {
	switch(button) {
	case GLFW_MOUSE_BUTTON_1: {
		switch(action) {
		case GLFW_PRESS: {
			program->drag_start_x = GLFW_TO_SCREEN_X(program->cursor_x) - program->data.image_renderer.value_offset_position_x;
			program->drag_start_y = GLFW_TO_SCREEN_Y(program->cursor_y) - program->data.image_renderer.value_offset_position_y;
			program->drag_mode = GL_TRUE;
		} break;

		case GLFW_RELEASE: {
			program->drag_mode = GL_FALSE;
		} break;

		default:
			return;
		}
	} break;

	default:
		return;
	}
}

#define ZOOM_INC 1.1f
#define ZOOM_DEC (1.0f / 1.1f)
#define ZOOM_MAX 5.0f
#define ZOOM_MIN 1.0f
void glfw_scroll_cb(GLFWwindow *w, double x, double y) {
	if (y == 0.0) return;

	GLfloat at_x = GLFW_TO_SCREEN_X(program->cursor_x);
	GLfloat at_y = GLFW_TO_SCREEN_Y(program->cursor_y);
	GLfloat amt = 0.0f;

	// Complicated clamping.
	if (y > 0.0) {
		if (program->data.image_renderer.value_scale >= ZOOM_MAX || ZOOM_INC >= ZOOM_MAX / program->data.image_renderer.value_scale) {
			amt = ZOOM_MAX / program->data.image_renderer.value_scale;
		} else amt = ZOOM_INC;
	} else {
		if (program->data.image_renderer.value_scale <= ZOOM_MIN || ZOOM_DEC <= ZOOM_MIN / program->data.image_renderer.value_scale) {
			amt = ZOOM_MIN / program->data.image_renderer.value_scale;
		} else amt = ZOOM_DEC;
	}
	
	// Zoom from mouse cursor instead of origin (0, 0).
	program->data.image_renderer.value_scale *= amt;
	program->data.image_renderer.value_offset_position_x = at_x - (at_x - program->data.image_renderer.value_offset_position_x) * amt;
	program->data.image_renderer.value_offset_position_y = at_y - (at_y - program->data.image_renderer.value_offset_position_y) * amt;
	program->drag_start_x = at_x - program->data.image_renderer.value_offset_position_x;
	program->drag_start_y = at_y - program->data.image_renderer.value_offset_position_y;
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

int clean_exit(int code) {
	if (!program) return code;

	// Clean up background.
	if (program->data.background_renderer.vert_source)
		free((void *)program->data.background_renderer.vert_source);
	if (program->data.background_renderer.frag_source)
		free((void *)program->data.background_renderer.frag_source);
	if (program->data.background_renderer.texdata)
		stbi_image_free(program->data.background_renderer.texdata);

	// Clean up image.
	if (program->data.image_renderer.vert_source)
		free((void *)program->data.image_renderer.vert_source);
	if (program->data.image_renderer.frag_source)
		free((void *)program->data.image_renderer.frag_source);
	if (program->data.image_renderer.texdata)
		stbi_image_free(program->data.image_renderer.texdata);

	if (program->is_glfw) {
		if (program->instance) glfwDestroyWindow(program->instance);
		glfwTerminate();
	}

	free((void *)program);
	return code;
}
