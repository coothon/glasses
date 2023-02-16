#version 460 core
out vec4 FragColor;

layout (location = 0) uniform sampler2D tex;

in vec3 color;
in vec2 uv;

void main(void) {
    FragColor = texture(tex, uv);
}