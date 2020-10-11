#version 410 core

uniform sampler2D spriteTexture;

flat in vec4 color;
flat in int tex_id;

in vec2 uv;

layout(location = 0) out vec4 out_color;

void main() {
	out_color = color * texture(spriteTexture, uv);
}