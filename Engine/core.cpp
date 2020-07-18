#include "core.hpp"
#include "engine.hpp"

#include <iostream>

#include <glad/glad.h>

#define T_CASE(type) case NodeType::type: \
	return #type

using namespace std;

namespace Morpheus {

	template <typename T, bool reinterpret> 
	struct DisposableInterfaceGate;
	template <typename T, bool reinterpret> 
	struct UpdatableInterfaceGate;
	template <typename T, bool reinterpret> 
	struct InitializableInterfaceGate;

	template <typename T>
	struct DisposableInterfaceGate<T, true> {
		static IDisposable* get(ref<void>& p) {
			return p.reinterpretGet<T>();
		}
	};
	
	template <typename T>
	struct DisposableInterfaceGate<T, false> {
		static IDisposable* get(ref<void>& p) {
			return nullptr;
		}
	};
 	
	template <typename T>
	struct UpdatableInterfaceGate<T, true> {
		static IUpdatable* get(ref<void>& p) {
			return p.reinterpretGet<T>();
		}
	};
	
	template <typename T>
	struct UpdatableInterfaceGate<T, false> {
		static IUpdatable* get(ref<void>& p) {
			return nullptr;
		}
	};
	
	template <typename T>
	struct InitializableInterfaceGate<T, true> {
		static IInitializable* get(ref<void>& p) {
			return p.reinterpretGet<T>();
		}
	};
	
	template <typename T>
	struct InitializableInterfaceGate<T, false> {
		static IInitializable* get(ref<void>& p) {
			return nullptr;
		}
	};
	
	bool NodeMetadata::renderable[(uint32_t)NodeType::END];
	bool NodeMetadata::pooled[(uint32_t)NodeType::END];
	bool NodeMetadata::content[(uint32_t)NodeType::END];
	NodeType NodeMetadata::proxyToPrototype[(uint32_t)NodeType::END];
	NodeType NodeMetadata::prototypeToProxy[(uint32_t)NodeType::END];
	DisposableConverter NodeMetadata::disposableConv[(uint32_t)NodeType::END];
	UpdatableConverter NodeMetadata::updatableConv[(uint32_t)NodeType::END];
	InitializableConverter NodeMetadata::initializableConv[(uint32_t)NodeType::END];

	template <NodeType iType> void NodeMetadata::init_() {
		pooled[(uint32_t)iType] = IS_POOLED_<iType>::RESULT;
		renderable[(uint32_t)iType] = IS_RENDERABLE_<iType>::RESULT;
		content[(uint32_t)iType] = IS_CONTENT_<iType>::RESULT;
		prototypeToProxy[(uint32_t)iType] = PROTOTYPE_TO_PROXY_<iType>::RESULT;
		proxyToPrototype[(uint32_t)iType] = PROXY_TO_PROTOTYPE_<iType>::RESULT;

		typedef typename OWNER_TYPE_<iType>::RESULT owner_t;

		disposableConv[(uint32_t)iType] = &DisposableInterfaceGate<owner_t, std::is_convertible<owner_t*, IDisposable*>::value>::get;
		updatableConv[(uint32_t)iType] = &UpdatableInterfaceGate<owner_t, std::is_convertible<owner_t*, IUpdatable*>::value>::get;
		initializableConv[(uint32_t)iType] = &InitializableInterfaceGate<owner_t, std::is_convertible<owner_t*, IInitializable*>::value>::get;
			
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
			T_CASE(TRANSFORM);
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
		vector<bool> isLast;
		iters.push(start.getChildren());

		if (start.outDegree() > 1)
			isLast.push_back(false);
		else
			isLast.push_back(true);

		while (!iters.empty()) {
			auto& it = iters.top();
			if (it.valid()) {
				Node n = it();

				for (int i = 0; i < isLast.size() - 1; ++i) {
					bool b = isLast[i];
					if (b)
						cout << " ";
					else
						cout << "|";

					for (uint32_t j = 0; j < depth_mul; ++j)
						cout << " ";
				}

				cout << "o";
				for (uint32_t j = 0; j < depth_mul; ++j)
					cout << "-";
				cout << "+ ";
				cout << nodeTypeString(descs[n].type) << endl;

				iters.push(n.getChildren());
				it.next();
				if (!it.valid())
					isLast[isLast.size() - 1] = true;
				if (n.outDegree() > 1)
					isLast.push_back(false);
				else
					isLast.push_back(true);
			}
			else {
				iters.pop();
				isLast.pop_back();
			}
		}
	}

	void printError(unsigned int err) {
		switch (err) {
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM!" << endl;
			break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE!" << endl;
			break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION!" << endl;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATOR!" << endl;
			break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY!" << endl;
			break;
		case GL_STACK_UNDERFLOW:
			cout << "GL_STACK_UNDERFLOW!" << endl;
			break;
		case GL_STACK_OVERFLOW:
			cout << "GL_STACK_OVERFLOW!" << endl;
			break;
		}
		assert(err == GL_NO_ERROR);
	}

	void init(Node& node, NodeGraph* graph, Node& updater) {
		auto desc = graph->desc(node);

		// Content doesn't belong to a scene and therefore shouldn't be initializable
		if (NodeMetadata::isContent(desc->type))
			return;

		// Initialize all children
		for (auto it = node.getChildren(); it.valid(); it.next()) {
			auto child = it();
			init(child);
		}

		// Initialize node after all its children have been initialized
		auto initInterface = getInterface<IInitializable>(*desc);
		if (initInterface)
			initInterface->init(node);

		// If this node is updatable, register it with the updater
		auto updateInterface = getInterface<IUpdatable>(*desc);
		if (updateInterface)
			updater.addChild(node);
	}

	void init(Node node)
	{
		auto graph_ = graph();
		auto updaterNode = (*graph_)[updater()->handle()];
		init(node, graph_, updaterNode);
	}
}