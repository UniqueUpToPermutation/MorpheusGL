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
#define IS_DISPOSABLE(nodeType)		NodeFlags<nodeType>::disposable

#define HANDLE_INVALID 0

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;
	
	class IDisposable {
	public:
		virtual void dispose() = 0;
	};

	struct BoundingBox {
		glm::vec3 mLower;
		glm::vec3 mUpper;
	};

	enum class NodeType : uint32_t {
		ENGINE,

		// All nodes that are found inside of a scene
		SCENE_BEGIN,
		EMPTY,
		SCENE_ROOT,
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

	template <NodeType nodeType>
	struct NodeFlags {
		enum {
			pooled = false,
			content = false,
			sceneChild = false,
			disposable = false
		};
	};

	template <>
	struct NodeFlags<NodeType::ENGINE> {
		enum {
			pooled = false,
			content = false,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::EMPTY> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::SCENE_ROOT> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::LOGIC> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::TRANSFORM> {
		enum {
			pooled = true,
			content = false,
			sceneChild = true,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::REGION> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::BOUNDING_BOX> {
		enum {
			pooled = true,
			content = false,
			sceneChild = true,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::DYNAMIC_OBJECT_MANAGER> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::STATIC_OBJECT_MANAGER> {
		enum {
			pooled = false,
			content = false,
			sceneChild = true,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::GEOMETRY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::CONTENT_MANAGER> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::CONTENT_FACTORY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = true
		};
	};
	template <>
	struct NodeFlags<NodeType::MATERIAL> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::SHADER> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_1D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_2D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_3D> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::TEXTURE_2D_ARRAY> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::CUBE_MAP> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};
	template <>
	struct NodeFlags<NodeType::STATIC_MESH> {
		enum {
			pooled = false,
			content = true,
			sceneChild = false,
			disposable = false
		};
	};

	template <typename T>
	struct ref;

	class NodeMetadata {
	public:
		typedef void (*disposer)(ref<void>&);

	private:
		static bool pooled[(uint32_t)NodeType::END];
		static bool content[(uint32_t)NodeType::END];
		static bool sceneChild[(uint32_t)NodeType::END];
		static bool disposable[(uint32_t)NodeType::END];

		template <uint32_t iType> 
		static void init_();
		template <> 
		static void init_<(uint32_t)NodeType::END>();

	public:
		static void init();
		static inline bool isPooled(NodeType t) { return pooled[(uint32_t)t]; }
		static inline bool isContent(NodeType t) { return content[(uint32_t)t]; }
		static inline bool isSceneChild(NodeType t) { return sceneChild[(uint32_t)t]; }
		static inline bool isDisposable(NodeType t) { return disposable[(uint32_t)t]; }
	};

	template <typename OwnerType> struct OwnerToNode;
	template <NodeType> struct NodeToOwner;

	REGISTER_NODE_TYPE(Engine, NodeType::ENGINE);
	REGISTER_NODE_TYPE(ContentManager, NodeType::CONTENT_MANAGER);
	REGISTER_NODE_TYPE(IContentFactory, NodeType::CONTENT_FACTORY);
	REGISTER_NODE_TYPE(BoundingBox, NodeType::BOUNDING_BOX);
	REGISTER_NODE_TYPE(char, NodeType::EMPTY);
	REGISTER_NODE_TYPE(Transform, NodeType::TRANSFORM);

	template <>
	struct ref<void> {
	public:
		union voidrefunion {
			PoolHandle<void> mHandle;
			void* mPtr;
			inline voidrefunion() { }
			inline ~voidrefunion() { }
		} p;

		template <typename T> ref<T> as();

		inline ref() {
		}
		inline ref(void* ptr) {
			p.mPtr = ptr;
		}
		inline ref(PoolHandle<void>& h) {
			p.mHandle = h;
		}
		template <typename T>
		inline T* getAs();
	};

	template <typename T, bool pooled>
	struct ref_pool_gate;

	template <typename T>
	struct ref_pool_gate<T, true> {
		PoolHandle<T> mHandle;
		inline T* get() { return mHandle.get(); }
		inline void from(T* ptr) {
			assert(true);
		}
		inline void from(PoolHandle<T>& newH) {
			mHandle = newH;
		}
		inline void from(ref<void>& r) {
			mHandle = PoolHandle<T>(r.p.mHandle);
		}
		inline void to(ref<void>& r) {
			r.p.mHandle = PoolHandle<void>(mHandle);
		}
		inline static T* getAs(ref<void>& r) {
			return PoolHandle<T>(r.p).get();
		}
	};

	template <typename T>
	struct ref_pool_gate<T, false> {
		T* mPtr;
		inline T* get() { return mPtr; }
		inline void from(T* newPtr) {
			mPtr = newPtr;
		}
		inline void from(PoolHandle<T>& h) {
			assert(true);
		}
		inline void from(ref<void>& r) {
			mPtr = (T*)r.p.mPtr;
		}
		inline void to(ref<void>& r) {
			r.p.mPtr = mPtr;
		}
		inline static T* getAs(ref<void>& r) {
			static_cast<T*>(r.p.mPtr);
		}
	};

	template<typename T>
	inline T* Morpheus::ref<void>::getAs()
	{
		return ref_pool_gate<T, IS_POOLED(NODE_TYPE(T))>::getAs(*this);
	}

	template <typename T>
	struct ref {
	private:
		ref_pool_gate<T, IS_POOLED(NODE_TYPE(T))> mPoolGate;

	public:
		inline T* get() { return mPoolGate.get(); }
		inline T* operator->() { return mPoolGate.get(); }

		inline ref<void> asvoid() {
			ref<void> r;
			mPoolGate.to(r);
			return r;
		}

		inline ref(T* ptr) {
			mPoolGate.from(ptr);
		}

		inline ref(PoolHandle<T>& h) {
			mPoolGate.from(h);
		}

		inline ref(ref<void>& r) {
			mPoolGate.from(r);
		}

		friend struct ref<void>;
	};

	template<typename T>
	inline ref<T> ref<void>::as()
	{
		return ref<T>(*this);
	}

	inline void dispose(ref<void>& r) {
		static_cast<IDisposable*>(r.p.mPtr)->dispose();
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
		DigraphDataView<NodeData> mDescs;
		DigraphVertexLookupView<NodeHandle> mHandles;
		DigraphVertexLookupView<std::string> mNames;
		NodeHandle mLargestHandle;

	public:
		inline NodeDataView& descs() { 
			return mDescs; 
		}
		inline NodeData desc(const DigraphVertex& v) {
			return mDescs[v];
		}
		inline NodeHandleLookupView handles() {
			return mHandles;
		}
		inline NodeNameLookupView names() {
			return mNames;
		}
		inline DigraphVertex operator[](const NodeHandle handle) {
			return mHandles[handle];
		}
		inline DigraphVertex operator[](const std::string& name) {
			return mNames[name];
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = ref<void>(owner);
			auto v = createVertex();
			mDescs[v] = data;
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
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(mNames[parentName], v);
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
			DigraphVertex u = mHandles[parentHandle];
			createEdge(u, v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(OwnerType* owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(mNames[parentName], v);
			return v;
		}

		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = owner;
			auto v = createVertex();
			mDescs[v] = data;
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
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(mNames[parentName], v);
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
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline DigraphVertex addNode(ref<void> owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(mNames[parentName], v);
			return v;
		}

		inline DigraphVertex addNode(Engine* owner) {
			return addNode(owner, NodeType::ENGINE);
		}
		inline NodeHandle issueHandle(const DigraphVertex& vertex) {
			NodeHandle h = ++mLargestHandle;
			mHandles.set(vertex, h);
			return h;
		}
		inline void recallHandle(const NodeHandle handle) {
			mHandles.clear(handle);
		}
		inline void setName(const DigraphVertex& vertex, const std::string& name) {
			mNames.set(vertex, name);
		}
		inline void recallName(const std::string& name) {
			mNames.clear(name);
		}

		NodeGraph() : mLargestHandle(HANDLE_INVALID) {
			mDescs = createVertexData<NodeData>("desc");
			mHandles = createVertexLookup<NodeHandle>("handle");
			mNames = createVertexLookup<std::string>("name");
		}
	};

	struct Transform {
		glm::vec3 mTranslation;
		glm::vec3 mScale;
		glm::quat mRotation;
		glm::mat4 mCache;
	};
	
	enum class ErrorCode {
		SUCCESS,
		FAIL_GLFW_INIT,
		FAIL_GLFW_WINDOW_INIT
	};

	class Error {
	public:
		ErrorCode mCode;
		std::string mMessage;
		std::string mSource;

		bool isSuccess() const {
			return mCode == ErrorCode::SUCCESS;
		}

		std::string str() const {
			return mSource + ": " + mMessage;
		}

		inline explicit Error(const ErrorCode code) : mCode(code) { }
	};
}