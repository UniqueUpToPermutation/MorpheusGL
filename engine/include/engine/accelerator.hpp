#pragma once

#include <engine/core.hpp>

namespace Morpheus {
	class IAccelerator : public INodeOwner {
	public:
		virtual void invalidate(INodeOwner* node) = 0;
		virtual void rebuild() = 0;
	};
	SET_NODE_ENUM(IAccelerator, ACCELERATOR);
}