#include "core.hpp"
#include "engine.hpp"

#include <iostream>

#define T_CASE(type) case NodeType::type: \
	return #type

using namespace std;

namespace Morpheus {

	bool NodeMetadata::renderable[(uint32_t)NodeType::END];
	bool NodeMetadata::disposable[(uint32_t)NodeType::END];
	bool NodeMetadata::pooled[(uint32_t)NodeType::END];
	bool NodeMetadata::content[(uint32_t)NodeType::END];
	bool NodeMetadata::updatable[(uint32_t)NodeType::END];
	NodeType NodeMetadata::proxyToPrototype[(uint32_t)NodeType::END];
	NodeType NodeMetadata::prototypeToProxy[(uint32_t)NodeType::END];

	template <NodeType iType> void NodeMetadata::init_() {
		pooled[(uint32_t)iType] = IS_POOLED_<iType>::RESULT;
		renderable[(uint32_t)iType] = IS_RENDERABLE_<iType>::RESULT;
		updatable[(uint32_t)iType] = IS_UPDATABLE_<iType>::RESULT;
		content[(uint32_t)iType] = IS_CONTENT_<iType>::RESULT;
		disposable[(uint32_t)iType] = IS_DISPOSABLE_<iType>::RESULT;
		prototypeToProxy[(uint32_t)iType] = PROTOTYPE_TO_PROXY_<iType>::RESULT;
		proxyToPrototype[(uint32_t)iType] = PROXY_TO_PROTOTYPE_<iType>::RESULT;
			
		init_<(NodeType)((uint32_t)iType + 1)>();
	}

	template <> void NodeMetadata::init_<NodeType::END>() {
	}

	void NodeMetadata::init() {
		init_<NodeType::START>();
	}

	std::string nodeTypeString(NodeType t) {
		switch (t) {
			T_CASE(START);
			T_CASE(ENGINE);
			T_CASE(RENDERER);
			T_CASE(UPDATER);
			T_CASE(SCENE_BEGIN);
			T_CASE(CAMERA);
			T_CASE(EMPTY);
			T_CASE(SCENE_ROOT);
			T_CASE(LOGIC);
			T_CASE(STATIC_TRANSFORM);
			T_CASE(DYNAMIC_TRANSFORM);
			T_CASE(REGION);
			T_CASE(BOUNDING_BOX);
			T_CASE(STATIC_OBJECT_MANAGER);
			T_CASE(DYNAMIC_OBJECT_MANAGER);
			T_CASE(MATERIAL_PROXY);
			T_CASE(GEOMETRY_PROXY);
			T_CASE(NANOGUI_SCREEN);
			T_CASE(SCENE_END);
			T_CASE(CONTENT_BEGIN);
			T_CASE(CONTENT_MANAGER);
			T_CASE(GEOMETRY);
			T_CASE(MATERIAL);
			T_CASE(SHADER);
			T_CASE(TEXTURE_2D);
			T_CASE(TEXTURE_1D);
			T_CASE(TEXTURE_3D);
			T_CASE(TEXTURE_2D_ARRAY);
			T_CASE(CUBE_MAP);
			T_CASE(STATIC_MESH);
			T_CASE(CONTENT_END);
			T_CASE(END);
		default:
			return "UNKNOWN";
		}
	}

	void print(Node start)
	{
		size_t depth_mul = 3;

		auto& descs = graph()->descs();

		cout << "+ " << nodeTypeString(descs[start].type) << endl;

		stack<DigraphVertexIteratorF> iters;
		iters.push(start.getOutgoingNeighbors());

		while (!iters.empty()) {
			auto& it = iters.top();
			if (it.valid()) {
				Node n = it();

				for (uint32_t i = 0; i < iters.size() - 1; ++i) {
					cout << "|";
					for (uint32_t j = 0; j < depth_mul; ++j)
						cout << " ";
				}

				cout << "o";
				for (uint32_t j = 0; j < depth_mul; ++j)
					cout << "-";
				cout << "+ ";
				cout << nodeTypeString(descs[n].type) << endl;

				iters.push(n.getOutgoingNeighbors());
				it.next();
			}
			else 
				iters.pop();
		}
	}
}