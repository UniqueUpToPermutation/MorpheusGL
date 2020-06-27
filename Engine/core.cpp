#include "core.hpp"

namespace Morpheus {

	bool NodeMetadata::sceneChild[(uint32_t)NodeType::END];
	bool NodeMetadata::disposable[(uint32_t)NodeType::END];
	bool NodeMetadata::content[(uint32_t)NodeType::END];
	bool NodeMetadata::pooled[(uint32_t)NodeType::END];

	template <uint32_t iType> void NodeMetadata::init_() {
		pooled[iType] = IS_POOLED((NodeType)iType);
		content[iType] = IS_CONTENT((NodeType)iType);
		sceneChild[iType] = IS_SCENE_CHILD((NodeType)iType);
		disposable[iType] = IS_DISPOSABLE((NodeType)iType);
			
		init_<iType + 1>();
	}

	template <> void NodeMetadata::init_<(uint32_t)NodeType::END>() {
	}

	void NodeMetadata::init() {
		init_<0>();
	}
}