#version 410 core

// Uses blit geometry
layout(location = 0) in vec2 position;

uniform vec3 eyePosition;
uniform mat4 view; 
uniform mat4 projection; 

out vec3 rayDir;

void main() {
	vec4 pos = vec4(position.x * 2.0 - 1.0, -position.y * 2.0 + 1.0, 0.999999, 1.0);
	gl_Position = pos;

	mat4 view_inv = inverse(view);
	mat4 proj_inv = inverse(projection);
	pos = proj_inv * pos;
	pos /= pos.w;
	pos = view_inv * pos;
	vec3 dir = pos.xyz - eyePosition;
	rayDir = normalize(dir);
}