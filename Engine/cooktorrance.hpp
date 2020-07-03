#pragma once

#include "shader.hpp"

namespace Morpheus {
	DEFINE_SHADER(CookTorranceShader) {
		SHADER_BODY;
		SHADER_UNIFORM(glm::vec3, mSpecularColor);
		SHADER_UNIFORM(float, mF0);
		SHADER_UNIFORM(float, mRoughness);
		SHADER_UNIFORM(float, mK);
		SHADER_UNIFORM(glm::vec3, mLightColor);
		SHADER_UNIFORM(float, mAmbientStrength);
		SHADER_UNIFORM(float, mLightIntensity);
		SHADER_UNIFORM(glm::vec3, mEyePosition);
		SHADER_UNIFORM(glm::vec3, mLightPosition);
	};
}