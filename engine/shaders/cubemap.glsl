#version 450 core

#define FACE_POSITIVE_X 0
#define FACE_NEGATIVE_X 1
#define FACE_POSITIVE_Y 2
#define FACE_NEGATIVE_Y 3
#define FACE_POSITIVE_Z 4
#define FACE_NEGATIVE_Z 5

// Interface
float cubemapJacobian(vec2 surfacePosition);
vec3 cubemapGetDirection(vec2 surfacePosition, uint face);

float cubemapJacobian(vec2 surfacePosition) {
	float mag2 = dot(surfacePosition, surfacePosition) + 1.0;
	float mag = sqrt(mag2);
	return 1.0 / (mag2 * mag);
}

vec3 cubemapGetDirection(vec2 surfacePosition, uint face) {
	vec3 result;
	switch (face) {
		case FACE_POSITIVE_X:
			result.x = 1.0;
			result.z = -surfacePosition.x;
			result.y = -surfacePosition.y;
			break;
		case FACE_NEGATIVE_X:
			result.x = -1.0;
			result.z = surfacePosition.x;
			result.y = -surfacePosition.y;
			break;
		case FACE_POSITIVE_Y:
			result.x = surfacePosition.x;
			result.y = 1.0;
			result.z = surfacePosition.y;
			break;
		case FACE_NEGATIVE_Y:
			result.x = surfacePosition.x;
			result.y = -1.0;
			result.z = -surfacePosition.y;
			break;
		case FACE_POSITIVE_Z:
			result.x = surfacePosition.x;
			result.y = -surfacePosition.y;
			result.z = 1.0;
			break;
		case FACE_NEGATIVE_Z:
			result.x = -surfacePosition.x;
			result.y = -surfacePosition.y;
			result.z = -1.0;
			break;
	}
	return normalize(result);
}