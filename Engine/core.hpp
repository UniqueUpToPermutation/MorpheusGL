#pragma once

#include "json.hpp"
#include "digraph.hpp"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <string>

#define REGISTER_NODE_TYPE(ownerType, nodeType) template <> struct OwnerToNode<ownerType> { public: enum { type = nodeType }; };
#define NODE_TYPE(ownerType) (NodeType)OwnerToNode<ownerType>::type

#define HANDLE_INVALID 0

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;

	enum class NodeType {
		ENGINE,

		// All nodes that are found inside of a scene
		SCENE_BEGIN,
		EMPTY,
		SCENE_ROOT,
		ENTITY,
		LOGIC,
		TRANSFORM,
		REGION,
		BOUNDING_BOX,
		STATIC_OBJECT_MANAGER,
		DYNAMIC_OBJECT_MANAGER,
		SCENE_END,

		// All nodes that are children of the content manager
		CONTENT_BEGIN,
		CONTENT_MANAGER,
		CONTENT_FACTORY,
		GEOMETRY,
		MATERIAL,
		SHADER,
		TEXTURE_2D,
		TEXTURE_1D,
		TEXTURE_3D,
		TEXTURE_2D_ARRAY,
		CUBE_MAP,
		STATIC_MESH,
		CONTENT_END
	};

	template <typename OwnerType> struct OwnerToNode;
	REGISTER_NODE_TYPE(Engine, NodeType::ENGINE);
	REGISTER_NODE_TYPE(ContentManager, NodeType::CONTENT_MANAGER);

	union OwnerRef {
		uint32_t offset;
		void* ptr;
	};

	struct NodeData {
		NodeType type;
		OwnerRef owner;
	};

	typedef uint32_t NodeHandle;
	typedef DigraphDataView<NodeData> NodeDataView;
	typedef DigraphVertexLookupView<NodeHandle> NodeHandleLookupView;
	typedef DigraphVertexLookupView<std::string> NodeNameLookupView;

	class NodeGraph : public Digraph {
	private:
		// Descriptions of the types and owners of each node
		DigraphDataView<NodeData> descs_;
		DigraphVertexLookupView<NodeHandle> handles_;
		DigraphVertexLookupView<std::string> names_;
		NodeHandle largest_handle_;

	public:
		inline NodeDataView& descs() { 
			return descs_; 
		}
		inline NodeData desc(const DigraphVertex& v) {
			return descs_[v];
		}
		inline NodeHandleLookupView handles() {
			return handles_;
		}
		inline NodeNameLookupView names() {
			return names_;
		}
		inline DigraphVertex operator[](const NodeHandle handle) {
			return handles_[handle];
		}
		inline DigraphVertex operator[](const std::string& name) {
			return names_[name];
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner.ptr = (void*)owner;
			auto v = createVertex();
			descs_[v] = data;
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(handles_[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(names_[parentName], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			DigraphVertex u = handles_[parentHandle];
			createEdge(u, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(names_[parentName], v);
			return v;
		}



		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = owner;
			auto v = createVertex();
			descs_[v] = data;
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, NodeType type, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(handles_[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(names_[parentName], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(handles_[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerRef owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(names_[parentName], v);
			return v;
		}

		inline DigraphVertex addNode(Engine* owner) {
			return addNode(owner, NodeType::ENGINE);
		}
		inline NodeHandle issueHandle(const DigraphVertex& vertex) {
			NodeHandle h = ++largest_handle_;
			handles_.set(vertex, h);
			return h;
		}
		inline void recallHandle(const NodeHandle handle) {
			handles_.clear(handle);
		}
		inline void setName(const DigraphVertex& vertex, const std::string& name) {
			names_.set(vertex, name);
		}
		inline void recallName(const std::string& name) {
			names_.clear(name);
		}

		NodeGraph() : largest_handle_(HANDLE_INVALID) {
			descs_ = createVertexData<NodeData>("desc");
			handles_ = createVertexLookup<NodeHandle>("handle");
			names_ = createVertexLookup<std::string>("name");
		}
	};

	struct Transform {
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rotation;
		glm::mat4 cache;
	};
	
	enum class ErrorCode {
		SUCCESS,
		FAIL_GLFW_INIT,
		FAIL_GLFW_WINDOW_INIT
	};

	class Error {
	public:
		ErrorCode code;
		std::string message;
		std::string source;

		bool isSuccess() const {
			return code == ErrorCode::SUCCESS;
		}

		std::string str() const {
			return source + ": " + message;
		}

		inline explicit Error(const ErrorCode code) : code(code) { }
	};
}