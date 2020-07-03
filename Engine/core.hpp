#pragma once

#include "json.hpp"
#include "digraph.hpp"
#include "pool.hpp"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <string>

#define HANDLE_INVALID 0

#define SET_POOLED(Type, bPooled) template <> struct IS_POOLED_<NodeType::Type>	\
	{ static const bool RESULT = bPooled; }

#define SET_CONTENT(Type, bIsContent) template <> struct IS_CONTENT_<NodeType::Type>	\
	{ static const bool RESULT = bIsContent; }

#define SET_SCENE_CHILD(Type, bIsChild) template <> struct IS_SCENE_CHILD_<NodeType::Type>	\
	{ static const bool RESULT = bIsChild; }

#define SET_DISPOSABLE(Type, bIsDisposable) template <> struct IS_DISPOSABLE_<NodeType::Type>	\
	{ static const bool RESULT = bIsDisposable; }

#define SET_NODE_TYPE(OwnerType, Type) template<> struct NODE_TYPE_<OwnerType> \
	{ static const NodeType RESULT = NodeType::Type; }; \
	 template <> struct OWNER_TYPE_<NodeType::Type> \
	{ typedef OwnerType RESULT; }

#define SET_BASE_TYPE(SubType, BaseType) template<> struct BASE_TYPE_<SubType> \
	{ typedef BaseType RESULT; }

#define NODE_TYPE(OwnerType) NODE_TYPE_<typename BASE_TYPE_<OwnerType>::RESULT>::RESULT

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;

	typedef uint32_t NodeHandle;
	
	class IDisposable {
	public:
		virtual void dispose() = 0;
	};

	enum class RendererType {
		FORWARD
	};

	class IRenderer : public IDisposable {
	public:
		virtual void init() = 0;
		virtual void draw(DigraphVertex& scene) = 0;
		virtual NodeHandle handle() const = 0;
		virtual RendererType getType() const = 0;
	};

	struct BoundingBox {
		glm::vec3 mLower;
		glm::vec3 mUpper;
	};

	enum class NodeType : uint32_t {
		START,

		ENGINE,
		RENDERER,

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
	
	template <typename T>
	struct BASE_TYPE_ {
		typedef T RESULT;
	};
	template <typename T>
	struct NODE_TYPE_ {
		static const NodeType RESULT = NodeType::END;
	};
	template <NodeType>
	struct OWNER_TYPE_;

	template <NodeType>
	struct IS_POOLED_ {
		static const bool RESULT = false;
	};
	template <NodeType> 
	struct IS_CONTENT_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_SCENE_CHILD_ {
		static const bool RESULT = true;
	};
	template <NodeType>
	struct IS_DISPOSABLE_ {
		static const bool RESULT = true;
	};

	// IS_POOLED Flag
	SET_POOLED(ENGINE, false);
	SET_POOLED(RENDERER, false);
	SET_POOLED(EMPTY, false);
	SET_POOLED(LOGIC, false);
	SET_POOLED(TRANSFORM, true);
	SET_POOLED(REGION, false);
	SET_POOLED(BOUNDING_BOX, true);
	SET_POOLED(STATIC_OBJECT_MANAGER, false);
	SET_POOLED(DYNAMIC_OBJECT_MANAGER, false);

	SET_POOLED(CONTENT_MANAGER, false);
	SET_POOLED(CONTENT_FACTORY, false);
	SET_POOLED(GEOMETRY, false);
	SET_POOLED(MATERIAL, false);
	SET_POOLED(SHADER, false);
	SET_POOLED(TEXTURE_1D, false);
	SET_POOLED(TEXTURE_2D, false);
	SET_POOLED(TEXTURE_3D, false);
	SET_POOLED(CUBE_MAP, false);
	SET_POOLED(TEXTURE_2D_ARRAY, false);
	SET_POOLED(STATIC_MESH, false);

	// IS_CONTENT Flag
	SET_CONTENT(ENGINE, false);
	SET_CONTENT(RENDERER, false);
	SET_CONTENT(EMPTY, false);
	SET_CONTENT(LOGIC, false);
	SET_CONTENT(TRANSFORM, false);
	SET_CONTENT(REGION, false);
	SET_CONTENT(BOUNDING_BOX, false);
	SET_CONTENT(STATIC_OBJECT_MANAGER, false);
	SET_CONTENT(DYNAMIC_OBJECT_MANAGER, false);

	SET_CONTENT(CONTENT_MANAGER, true);
	SET_CONTENT(CONTENT_FACTORY, true);
	SET_CONTENT(GEOMETRY, true);
	SET_CONTENT(MATERIAL, true);
	SET_CONTENT(SHADER, true);
	SET_CONTENT(TEXTURE_1D, true);
	SET_CONTENT(TEXTURE_2D, true);
	SET_CONTENT(TEXTURE_3D, true);
	SET_CONTENT(CUBE_MAP, true);
	SET_CONTENT(TEXTURE_2D_ARRAY, true);
	SET_CONTENT(STATIC_MESH, true);

	// IS_SCENE_CHILD Flag
	SET_SCENE_CHILD(ENGINE, false);
	SET_SCENE_CHILD(RENDERER, false);
	SET_SCENE_CHILD(EMPTY, true);
	SET_SCENE_CHILD(LOGIC, true);
	SET_SCENE_CHILD(TRANSFORM, true);
	SET_SCENE_CHILD(REGION, true);
	SET_SCENE_CHILD(BOUNDING_BOX, true);
	SET_SCENE_CHILD(STATIC_OBJECT_MANAGER, true);
	SET_SCENE_CHILD(DYNAMIC_OBJECT_MANAGER, true);

	SET_SCENE_CHILD(CONTENT_MANAGER, false);
	SET_SCENE_CHILD(CONTENT_FACTORY, false);
	SET_SCENE_CHILD(GEOMETRY, false);
	SET_SCENE_CHILD(MATERIAL, false);
	SET_SCENE_CHILD(SHADER, false);
	SET_SCENE_CHILD(TEXTURE_1D, false);
	SET_SCENE_CHILD(TEXTURE_2D, false);
	SET_SCENE_CHILD(TEXTURE_3D, false);
	SET_SCENE_CHILD(CUBE_MAP, false);
	SET_SCENE_CHILD(TEXTURE_2D_ARRAY, false);
	SET_SCENE_CHILD(STATIC_MESH, false);

	// IS_DISPOSABLE Flag
	SET_DISPOSABLE(ENGINE, false);
	SET_DISPOSABLE(RENDERER, true);
	SET_DISPOSABLE(EMPTY, false);
	SET_DISPOSABLE(LOGIC, true);
	SET_DISPOSABLE(TRANSFORM, false);
	SET_DISPOSABLE(REGION, true);
	SET_DISPOSABLE(BOUNDING_BOX, false);
	SET_DISPOSABLE(STATIC_OBJECT_MANAGER, true);
	SET_DISPOSABLE(DYNAMIC_OBJECT_MANAGER, true);

	SET_DISPOSABLE(CONTENT_MANAGER, true);
	SET_DISPOSABLE(CONTENT_FACTORY, true);
	SET_DISPOSABLE(GEOMETRY, false);
	SET_DISPOSABLE(MATERIAL, false);
	SET_DISPOSABLE(SHADER, false);
	SET_DISPOSABLE(TEXTURE_1D, false);
	SET_DISPOSABLE(TEXTURE_2D, false);
	SET_DISPOSABLE(TEXTURE_3D, false);
	SET_DISPOSABLE(CUBE_MAP, false);
	SET_DISPOSABLE(TEXTURE_2D_ARRAY, false);
	SET_DISPOSABLE(STATIC_MESH, false);

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

		template <NodeType iType> 
		static void init_();
		template <> 
		static void init_<NodeType::END>();

	public:
		static void init();
		static inline bool isPooled(NodeType t) { return pooled[(uint32_t)t]; }
		static inline bool isContent(NodeType t) { return content[(uint32_t)t]; }
		static inline bool isSceneChild(NodeType t) { return sceneChild[(uint32_t)t]; }
		static inline bool isDisposable(NodeType t) { return disposable[(uint32_t)t]; }
	};

	SET_NODE_TYPE(Engine, ENGINE);
	SET_NODE_TYPE(ContentManager, CONTENT_MANAGER);
	SET_NODE_TYPE(IContentFactory, CONTENT_FACTORY);
	SET_NODE_TYPE(BoundingBox, BOUNDING_BOX);
	SET_NODE_TYPE(char, EMPTY);
	SET_NODE_TYPE(Transform, TRANSFORM);
	SET_NODE_TYPE(IRenderer, RENDERER);

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
			return static_cast<T*>(r.p.mPtr);
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
		ref_pool_gate<T, IS_POOLED_<NODE_TYPE_<typename 
			BASE_TYPE_<T>::RESULT>::RESULT>::RESULT> mPoolGate;

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
		inline DigraphVertex addNode(ref<void> owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = owner;
			auto v = createVertex();
			mDescs[v] = data;
			return v;
		}
		inline DigraphVertex addNode(ref<void> owner, NodeType type, DigraphVertex& parent) {
			auto v = addNode(owner, type);
			createEdge(parent, v);
			return v;
		}
		inline DigraphVertex addNode(ref<void> owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode(owner, type);
			auto p = mHandles[parentHandle];
			createEdge(p, v);
			return v;
		}
		inline DigraphVertex addNode(ref<void> owner, NodeType type, const std::string& parentName) {
			auto v = addNode(owner, type);
			auto p = mNames[parentName];
			createEdge(p, v);
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