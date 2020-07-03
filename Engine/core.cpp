#include "core.hpp"

namespace Morpheus {

	bool NodeMetadata::sceneChild[(uint32_t)NodeType::END];
	bool NodeMetadata::disposable[(uint32_t)NodeType::END];
	bool NodeMetadata::pooled[(uint32_t)NodeType::END];

	template <NodeType iType> void NodeMetadata::init_() {
		pooled[(uint32_t)iType] = IS_POOLED_<iType>::RESULT;
		sceneChild[(uint32_t)iType] = IS_SCENE_CHILD_<iType>::RESULT;
		disposable[(uint32_t)iType] = IS_DISPOSABLE_<iType>::RESULT;
		prototypeToInstance[(uint32_t)iType] = PROTOTYPE_TO_BASE_<iType>::RESULT;
		instanceToPrototype[(uint32_t)iType] = INSTANCE_TO_PROTOTYPE_<iType>::RESULT;
			
		init_<(NodeType)((uint32_t)iType + 1)>();
	}

	template <> void NodeMetadata::init_<NodeType::END>() {
	}

	void NodeMetadata::init() {
		init_<NodeType::START>();
	}
}