#pragma once

#include "shader.hpp"

namespace Morpheus {
	DEFINE_SHADER(BasicShader) {
		SHADER_BODY;
		SHADER_UNIFORM(glm::vec3, color);
	};
}