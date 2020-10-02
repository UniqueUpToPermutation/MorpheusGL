#version 410 core

// Input the UV coordinates
in vec2 uv;

// The texture to blit
uniform sampler2D blitTexture;

// Pixel color to write to screen
out vec4 outColor;

void main() {
	outColor = vec4(texture(blitTexture, uv).rgb, 1.0);
}