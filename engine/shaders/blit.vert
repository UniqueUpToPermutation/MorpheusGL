#version 410 core

layout(location = 0) in vec2 position;

// Lower bound for the blit rectangle in clip space
uniform vec2 lower;
// Upper bound for the blit rectangle in clip space
uniform vec2 upper;

out vec2 uv;

void main() {
	vec2 transformed = lower + position * (upper - lower);
	gl_Position = vec4(transformed, 0.0, 1.0); 
	uv = position;
}