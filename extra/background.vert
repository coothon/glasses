#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aUV;

uniform vec2 window_size;
uniform vec2 tex_size;
uniform vec2 offset_position;
uniform float scale;

out vec3 color;
out vec2 uv;

vec2 screen_project(vec2 pos) {
	return (pos / window_size) * 2.0 - 1.0;
}

void main(void) {
	vec2 tex_ratio = aPos * tex_size;
	color = aCol;
	uv = screen_project((tex_ratio * scale) + (window_size * 0.5) + offset_position);
	gl_Position = vec4(aPos * 2.0, 0.0, 1.0);
}
