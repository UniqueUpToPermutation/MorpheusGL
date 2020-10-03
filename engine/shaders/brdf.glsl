#version 410 core

// Includes
#pragma include "hammersley.glsl"
vec2 hammersley(uint i, const uint sampleCount);

#pragma include "ggx.glsl"
vec3 importanceSampleGGX(vec2 Xi, float Roughness, vec3 N);

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float G_smith(float Roughness, float NoV, float NoL) {
	float r = Roughness;
	float k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(NoV, k) * gaSchlickG1(NoL, k);
}

// Integrate the environment BRDF for the Cook Torrance kernel
vec2 integrateBRDF(float Roughness, float NoV, const uint NumSamples) {
	vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V;
	V.x = sqrt(1.0 - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;

	for (uint i = 0; i < NumSamples; i++)
	{
		vec2 Xi = hammersley(i, NumSamples);
		vec3 H = importanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(L.z, 0.0, 1.0);
		float NoH = clamp(H.z, 0.0, 1.0);
		float VoH = clamp(dot(V, H), 0.0, 1.0);
		if (NoL > 0)
		{
			float G = G_smith(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return vec2(A, B) / NumSamples;
}
