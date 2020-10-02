#version 450 core

#pragma include "hammersley.glsl"
vec2 hammersley(uint i, uint sampleCount);

const float PI = 3.1415926535897932384626433832795;

// Importance sample from the ggx distribution
vec3 importanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);

    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

// Prefilter an input environment map by convolving with the GGX distribution
vec3 ggxPrefilterEnvMap(samplerCube inputTexture, float Roughness, vec3 R)
{
	const uint NumSamples = 1024;

	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3(0.0, 0.0, 0.0);
	float TotalWeight = 0.0;

	for (uint i = 0; i < NumSamples; i++)
	{
		vec2 Xi = hammersley(i, NumSamples);
		vec3 H = importanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(dot(N, L), 0.0, 1.0);

		if (NoL > 0)
		{
			PrefilteredColor += textureLod(inputTexture, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}