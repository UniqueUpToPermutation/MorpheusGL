#pragma once

#include <glad/glad.h>

namespace Morpheus {
	enum class BlendMode {
		NONE,
		ADDITIVE,
		MULTIPLICATIVE,
		ALPHA
	};

	void setBlendMode(BlendMode mode);
}