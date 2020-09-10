#version 330 core

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;

uniform sampler2D albedo;
uniform samplerCube skybox;

out vec4 outColor;

void main()
{
	// outColor = texture(albedo, vTexcoords);
	outColor = texture(skybox, vNormal);
}