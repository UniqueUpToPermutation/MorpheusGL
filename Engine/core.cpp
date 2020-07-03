#include "core.hpp"

namespace Morpheus {

	bool NodeMetadata::sceneChild[(uint32_t)NodeType::END];
	bool NodeMetadata::disposable[(uint32_t)NodeType::END];
	bool NodeMetadata::content[(uint32_t)NodeType::END];
	bool NodeMetadata::pooled[(uint32_t)NodeType::END];

	template <NodeType iType> void NodeMetadata::init_() {
		pooled[(uint32_t)iType] = IS_POOLED_<iType>::RESULT;
		content[(uint32_t)iType] = IS_CONTENT_<iType>::RESULT;
		sceneChild[(uint32_t)iType] = IS_SCENE_CHILD_<iType>::RESULT;
		disposable[(uint32_t)iType] = IS_DISPOSABLE_<iType>::RESULT;
			
		init_<(NodeType)((uint32_t)iType + 1)>();
	}

	template <> void NodeMetadata::init_<NodeType::END>() {
	}

	void NodeMetadata::init() {
		init_<NodeType::START>();
	}
}