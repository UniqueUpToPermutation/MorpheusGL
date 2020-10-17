#version 410 core

// Input the UV coordinates
in vec2 uv;

// The texture to blit
uniform sampler2D blitTexture;

// Pixel color to write to screen
out vec4 outColor;

uniform float gamma     = 2.2;
uniform float exposure  = 1.0;
uniform float pureWhite = 1.0;

void main()
{
	vec3 color = texture(blitTexture, uv).rgb * exposure;

	// Reinhard tonemapping operator.
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	vec3 mappedColor = (mappedLuminance / luminance) * color;

	// Gamma correction.
	outColor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);
}