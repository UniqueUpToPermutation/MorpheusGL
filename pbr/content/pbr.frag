#version 450 core

in vec2 vTexcoords;
in vec3 vNormal;
in vec3 vTanget;
in vec3 vPosition;

#define SH_C_0 0.28209479177
#define SH_C_1 0.4886025119
#define SH_C_2n2 1.09254843059
#define SH_C_2n1 SH_C_2n2
#define SH_C_20 0.31539156525
#define SH_C_21 SH_C_2n2
#define SH_C_22 0.54627421529

#define SH_COEFFS 9

uniform vec3 diffuseIrradianceSH[9];
uniform float roughness = 0.0;

layout(binding = 0) uniform samplerCube specularEnvMap;

out vec4 outColor;

vec3 calcDiffuseIrradiance(vec3 normal) {
	return (
		SH_C_0 * diffuseIrradianceSH[0] +
		SH_C_1 * normal.y * diffuseIrradianceSH[1] + 
		SH_C_1 * normal.z * diffuseIrradianceSH[2] + 
		SH_C_1 * normal.x * diffuseIrradianceSH[3] + 
		SH_C_2n2 * normal.x * normal.y * diffuseIrradianceSH[4] +
		SH_C_2n1 * normal.y * normal.z * diffuseIrradianceSH[5] +
		SH_C_20 * (-normal.x * normal.x - 
			normal.y * normal.y + 2.0 * normal.z * normal.z) * diffuseIrradianceSH[6] +
		SH_C_21 * normal.z * normal.x * diffuseIrradianceSH[7] + 
		SH_C_22 * (normal.x * normal.x - normal.y * normal.y) * diffuseIrradianceSH[8]
    );
}

void main()
{
	vec3 dir = normalize(vNormal);
	//outColor = vec4(calcDiffuseIrradiance(dir), 1.0);
	outColor = textureLod(specularEnvMap, dir, roughness * float(textureQueryLevels(specularEnvMap) - 1));
}