#pragma once

#include "json.hpp"
#include "digraph.hpp"
#include "pool.hpp"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <string>

#define REGISTER_NODE_TYPE(ownerType, nodeType) template <> struct OwnerToNode<ownerType> { enum { type = nodeType }; }; \
	template <> struct NodeToOwner<nodeType> { typedef ownerType type; };

#define NODE_TYPE(ownerType) (NodeType)OwnerToNode<ownerType>::type
#define OWNER_TYPE(nodeType) NodeToOwner<nodeType>::type

#define IS_POOLED(nodeType)			NodeFlags<nodeType>::pooled
#define IS_CONTENT(nodeType)		NodeFlags<nodeType>::content
#define IS_SCENE_CHILD(nodeType)	NodeFlags<nodeType>::sceneChild

#define HANDLE_INVALID 0

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;
	
	struct BoundingBox {
		glm::vec3 lower;
		glm::vec3 upper;
	};

	enum class NodeType : uint32_t {
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
		CONTENT_END,

		END
	};

	template <bool pooled, typename T>
	struct ConditionalPool;

	template <typename T>
	struct ConditionalPool<true, T> {
		Pool<T> pool;
	};

	template <typename T>
	struct ConditionalPool<false, T> {
	};

	template <NodeType nodeType>
	struct NodeFlags;
	template <>
	struct NodeFlags<NodeType::ENGINE> {
		enum {
			pooled = false,
			content = false,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::EMPTY> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::SCENE_ROOT> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::ENTITY> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::LOGIC> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::TRANSFORM> {
		enum {
			pooled = true,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::REGION> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
		};
	};
	template <>
	struct NodeFlags<NodeType::BOUNDING_BOX> {
		enum {
			pooled = true,
			content = false,
			sceneChild = true,
		};
	};
	template <>
	struct NodeFlags<NodeType::DYNAMIC_OBJECT_MANAGER> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::STATIC_OBJECT_MANAGER> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true
		};
	};
	template <>
	struct NodeFlags<NodeType::GEOMETRY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::CONTENT_MANAGER> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
		};
	};
	template <>
	struct NodeFlags<NodeType::CONTENT_FACTORY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::MATERIAL> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::SHADER> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_1D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_2D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_3D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_2D_ARRAY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::CUBE_MAP> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};
	template <>
	struct NodeFlags<NodeType::STATIC_MESH> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false
		};
	};

	template <typename OwnerType> struct OwnerToNode;
	template <NodeType> struct NodeToOwner;

	REGISTER_NODE_TYPE(Engine, NodeType::ENGINE);
	REGISTER_NODE_TYPE(ContentManager, NodeType::CONTENT_MANAGER);
	REGISTER_NODE_TYPE(IContentFactory, NodeType::CONTENT_FACTORY);
	REGISTER_NODE_TYPE(BoundingBox, NodeType::BOUNDING_BOX);
	REGISTER_NODE_TYPE(char, NodeType::EMPTY);
	REGISTER_NODE_TYPE(Transform, NodeType::TRANSFORM);

	template <typename T>
	struct ref;

	template <>
	struct ref<void> {
	public:
		union voidrefunion {
			PoolHandle<void> h;
			void* ptr;
			inline voidrefunion() { }
			inline ~voidrefunion() { }
		} p;

		template <typename T> ref<T> as();

		inline ref() {
		}
		inline ref(void* ptr) {
			p.ptr = ptr;
		}
		inline ref(PoolHandle<void>& h) {
			p.h = h;
		}
	};

	template <typename T, bool pooled>
	struct ref_pool_gate;

	template <typename T>
	struct ref_pool_gate<T, true> {
		PoolHandle<T> h;
		inline T* get() { return h.get(); }
		inline void from(T* ptr) {
			assert(true);
		}
		inline void from(PoolHandle<T>& newH) {
			h = newH;
		}
		inline void from(ref<void>& r) {
			h = PoolHandle<T>(r.p.h);
		}
		inline void to(ref<void>& r) {
			r.p.h = PoolHandle<void>(h);
		}
	};

	template <typename T>
	struct ref_pool_gate<T, false> {
		T* ptr;
		inline T* get() { return ptr; }
		inline void from(T* newPtr) {
			ptr = newPtr;
		}
		inline void from(PoolHandle<T>& h) {
			assert(true);
		}
		inline void from(ref<void>& r) {
			ptr = (T*)r.p.ptr;
		}
		inline void to(ref<void>& r) {
			r.p.ptr = ptr;
		}
	};

	template <typename T>
	struct ref {
	private:
		ref_pool_gate<T, IS_POOLED(NODE_TYPE(T))> poolGate;

	public:
		inline T* get() { return poolGate.get(); }
		inline T* operator->() { return poolGate.get(); }

		inline ref<void> asvoid() {
			ref<void> r;
			poolGate.to(r);
			return r;
		}

		inline ref(T* ptr) {
			poolGate.from(ptr);
		}

		inline ref(PoolHandle<T>& h) {
			poolGate.from(h);
		}

		inline ref(ref<void>& r) {
			poolGate.from(r);
		}

		friend struct ref<void>;
	};

	template<typename T>
	inline ref<T> ref<void>::as()
	{
		return ref<T>(*this);
	}

	struct NodeData {
		NodeType type;
		ref<void> owner;
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
			data.owner = ref<void>(owner);
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
		inline DigraphVertex addNode(ref<void> owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = owner;
			auto v = createVertex();
			descs_[v] = data;
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeType type, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(handles_[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(names_[parentName], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, DigraphVertex& parent) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(handles_[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, const std::string& parentName) {
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