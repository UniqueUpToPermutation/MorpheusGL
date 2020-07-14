#version 330 core

#include "cooktorrance.glsl"

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;
in vec3 vColor;

uniform vec3 specularColor = vec3(1, 1, 1);
uniform float F0 = 0.8;
uniform float roughness = 0.1;
uniform float k = 0.2;
uniform vec3 lightColor = vec3(1, 1, 1);
uniform float ambientStrength = 0.0;
uniform float lightIntensity = 1.0;
uniform vec3 eyePosition;
uniform vec3 lightDirection;

out vec3 outColor;

void main()
{
	vec3 diffuseColor = vec3(1.0, 1.0, 1.0);

	vec3 vViewDir = normalize(eyePosition - vPosition);
	vec3 vLightDir = - normalize(lightDirection);

	outColor = CookTorrance(diffuseColor,
		specularColor,
		vNormal,
		vLightDir,
		vViewDir,
		lightColor,
		F0,
		roughness,
		k);

	outColor = outColor * lightIntensity + diffuseColor * ambientStrength;
	outColor = clamp(outColor, 0.0, 1.0);
	outColor = vColor * outColor;
}