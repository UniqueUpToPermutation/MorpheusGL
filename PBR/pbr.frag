#version 330 core

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;

uniform sampler2D albedo;

out vec4 outColor;

void main()
{
	outColor = texture2D(albedo, vTexcoords);
}