#include <engine/blend.hpp>

namespace Morpheus {
	void setBlendMode(BlendMode mode) {
		switch (mode) {
		case BlendMode::NONE:
			glDisable(GL_BLEND);
			break;
		case BlendMode::ADDITIVE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case BlendMode::MULTIPLICATIVE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			break;
		case BlendMode::ALPHA:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	}
}