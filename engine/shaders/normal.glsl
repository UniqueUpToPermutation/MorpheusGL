#version 410 core

vec3 computeNormalFromMap(vec3 inputNormal, vec3 inputTangent, vec3 normalMapSample) {
	vec3 n = normalize(inputNormal);
	vec3 t = normalize(inputTangent);
	vec3 b = cross(n, t);

	vec3 n_map = normalMapSample * 2.0 - 1.0;
	return normalize(n_map.x * t + n_map.y * b + n_map.z * n);
}