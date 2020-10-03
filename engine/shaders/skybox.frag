#version 410 core

in vec3 rayDir;

uniform samplerCube skybox;

out vec4 color;

void main() {
	color = texture(skybox, rayDir);
}