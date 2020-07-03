﻿#version 330 core

#define PI 3.14159265

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;

uniform vec3 specularColor = vec3(1, 1, 1);
uniform float F0 = 0.8;
uniform float roughness = 0.1;
uniform float k = 0.2;
uniform vec3 lightColor = vec3(1, 1, 1);
uniform float ambientStrength = 0.0;
uniform float lightIntensity = 1.0;
uniform vec3 eyePosition;
uniform vec3 lightPosition;

out vec3 outColor;

vec3 CookTorrance(vec3 materialDiffuseColor,
	vec3 materialSpecularColor,
	vec3 normal,
	vec3 lightDir,
	vec3 viewDir,
	vec3 lightColor)
{
	float NdotL = max(0, dot(normal, lightDir));
	float Rs = 0.0;
	if (NdotL > 0) 
	{
		vec3 H = normalize(lightDir + viewDir);
		float NdotH = max(0, dot(normal, H));
		float NdotV = max(0, dot(normal, viewDir));
		float VdotH = max(0.000001, dot(lightDir, H));

		// Fresnel reflectance
		float F = pow(1.0 - VdotH, 5.0);
		F *= (1.0 - F0);
		F += F0;

		// Microfacet distribution by Beckmann
		float m_squared = roughness * roughness;
		float r1 = 1.0 / max(4.0 * m_squared * pow(NdotH, 4.0), 0.000001);
		float r2 = (NdotH * NdotH - 1.0) / max(m_squared * NdotH * NdotH, 0.000001);
		float D = r1 * exp(r2);

		// Geometric shadowing
		float two_NdotH = 2.0 * NdotH;
		float g1 = (two_NdotH * NdotV) / VdotH;
		float g2 = (two_NdotH * NdotL) / VdotH;
		float G = min(1.0, min(g1, g2));

		Rs = (F * D * G) / max(PI * NdotL * NdotV, 0.000001);
	}
	
	return materialDiffuseColor * lightColor * NdotL + lightColor * materialSpecularColor * NdotL * (k + Rs * (1.0 - k));
}

void main()
{
	vec3 diffuseColor = vec3(1.0, 1.0, 1.0);

	vec3 vViewDir = normalize(eyePosition - vPosition);
	vec3 vLightDir = normalize(lightPosition - vPosition);

	outColor = CookTorrance(diffuseColor,
		specularColor,
		vNormal,
		vLightDir,
		vViewDir,
		lightColor);

	outColor = outColor * lightIntensity + diffuseColor * ambientStrength;
	outColor = clamp(outColor, 0.0, 1.0);
}