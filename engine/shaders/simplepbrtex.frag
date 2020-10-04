#version 450 core

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

uniform vec3 environmentDiffuseSH[9];

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metalnessMap;

uniform vec3 eyePosition;

uniform samplerCube environmentSpecular;
uniform sampler2D environmentBRDF;

out vec4 outColor;

#pragma include "sphericalharmonics.glsl"
vec3 reconstructSH9(vec3 coeffs[9], vec3 normal);

#pragma include "normal.glsl"
vec3 computeNormalFromMap(vec3 inputNormal, vec3 inputTangent, vec3 normalMapSample);

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	vec3 normal_sample = texture(normalMap, vTexcoords).xyz;
	vec3 N = computeNormalFromMap(vNormal, vTanget, normal_sample);

	vec3 Lo = normalize(eyePosition - vPosition);
	vec3 Lr = reflect(-Lo, N);
	float NoR = clamp(dot(Lo, N), 0.0, 1.0);

	vec3 albedo_sample = texture(albedoMap, vTexcoords).rgb;
	float roughness_sample = texture(roughnessMap, vTexcoords).r;
	float metalness_sample = texture(metalnessMap, vTexcoords).r;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(Fdielectric, albedo_sample, metalness_sample);
	vec3 F = fresnelSchlick(F0, NoR);

	// Get diffuse contribution factor (as with direct lighting).
	vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness_sample);

	// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
	vec3 diffuseIBL = kd * albedo_sample * reconstructSH9(environmentDiffuseSH, N);

	// Sample pre-filtered specular reflection environment at correct mipmap level.
	int specularTextureLevels = textureQueryLevels(environmentSpecular);
	vec3 specularIrradiance = textureLod(environmentSpecular, Lr, roughness_sample * (specularTextureLevels - 1)).rgb;

	// Split-sum approximation factors for Cook-Torrance specular BRDF.
	vec2 specularBRDF = texture(environmentBRDF, vec2(roughness_sample, NoR)).rg;

	// Total specular IBL contribution.
	vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
	vec3 ambientLighting = diffuseIBL + specularIBL;

	outColor = vec4(ambientLighting, 1.0);
}