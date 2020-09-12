/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: core.hpp
*	Description: Defines all core functionality that the engine depends on.
*	This principally includes the scene graph, as well as metadata for different
*	node types in the scene graph.
*/

#pragma once

#include <engine/json.hpp>
#include <engine/digraph.hpp>
#include <engine/pool.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

#define HANDLE_INVALID 0

#define SET_POOLED(Type, bPooled) template <> struct IS_POOLED_<NodeType::Type>	\
	{ static constexpr bool RESULT = bPooled; }

#define SET_CONTENT(Type, bIsContent) template <> struct IS_CONTENT_<NodeType::Type> \
	{ static constexpr bool RESULT = bIsContent; }

#define SET_RENDERABLE(Type, bIsChild) template <> struct IS_RENDERABLE_<NodeType::Type>	\
	{ static constexpr bool RESULT = bIsChild; }

#define SET_DISPOSABLE(Type, bIsDisposable) template <> struct IS_DISPOSABLE_<NodeType::Type>	\
	{ static constexpr bool RESULT = bIsDisposable; }

#define SET_UPDATABLE(Type, bIsUpdateable) template <> struct IS_UPDATABLE_<NodeType::Type>	\
	{ static constexpr bool RESULT = bIsUpdateable; }

#define SET_INITIALIZABLE(Type, bIsInitializable) template <> struct IS_INITIALIZABLE_<NodeType::Type> \
	{ static constexpr bool RESULT = bIsInitializable; }

#define SET_NODE_ENUM(OwnerType, Type) template<> struct NODE_TYPE_<OwnerType> \
	{ static constexpr NodeType RESULT = NodeType::Type; }; \
	 template <> struct OWNER_TYPE_<NodeType::Type> \
	{ typedef OwnerType RESULT; }

#define SET_BASE_TYPE(SubType, BaseType) template<> struct BASE_TYPE_<SubType> \
	{ typedef BaseType RESULT; }

#define NODE_ENUM(OwnerType) Morpheus::NODE_TYPE_<typename BASE_TYPE_<OwnerType>::RESULT>::RESULT

#define DEF_PROXY(InstanceType, NodeType_) template<> struct PROXY_TO_PROTOTYPE_<NodeType::InstanceType> \
	{ static constexpr NodeType RESULT = NodeType::NodeType_; }; \
	template<> struct PROTOTYPE_TO_PROXY_<NodeType::NodeType_> \
	{ static constexpr NodeType RESULT = NodeType::InstanceType; } ;

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;
	class Updater;
	class Scene;
	class Camera;
	class GuiBase;
	class IRenderer;
	class Shader;
	class Material;
	class StaticMesh;
	class Geometry;
	class ILogic;

	typedef DigraphVertex Node;
	typedef uint32_t NodeHandle;

	
	/// Any class that should be a child of the updater and updated every frame.
	class IUpdatable {
	public:
		virtual void setEnabled(bool) = 0;
		virtual bool isEnabled() const = 0;
		virtual void update(double dt) = 0;
	};
	
	/// Any class that must be disposed before it can be dropped from the scene graph.
	class IDisposable {
	public:
		virtual void dispose() = 0;
	};
	
	/// Any class that should be initialized before a scene becomes active.
	/// This includes logic nodes, etc.
	class IInitializable {
	public:
		virtual void init(Node node) = 0;
	};
	
	enum class RendererType {
		FORWARD
	};

	class IRenderer;

	struct BoundingBox {
		glm::vec3 mLower;
		glm::vec3 mUpper;
	};

	enum class NodeType : uint32_t {
		START,

		ENGINE,
		RENDERER,
		UPDATER,

		// All nodes that are found inside of a scene
		SCENE_BEGIN,
		CAMERA,
		EMPTY,
		SCENE_ROOT,
		LOGIC,
		TRANSFORM,
		REGION,
		BOUNDING_BOX,
		STATIC_OBJECT_MANAGER,
		DYNAMIC_OBJECT_MANAGER,
		NANOGUI_SCREEN,
		SKYBOX,
		SCENE_END,

		// All nodes that are children of the content manager
		CONTENT_BEGIN,
		CONTENT_MANAGER,
		GEOMETRY,
		HALF_EDGE_GEOMETRY,
		MATERIAL,
		SHADER,
		TEXTURE,
		STATIC_MESH,
		CONTENT_END,
		SAMPLER,

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
	struct OWNER_TYPE_ {
		typedef void RESULT;
	};

	template <NodeType>
	struct IS_POOLED_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_RENDERABLE_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_UPDATABLE_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_DISPOSABLE_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_CONTENT_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_INITIALIZABLE_ {
		static const bool RESULT = false;
	};
	template <NodeType t>
	struct PROXY_TO_PROTOTYPE_ {
		static const NodeType RESULT = t;
	};
	template <NodeType t>
	struct PROTOTYPE_TO_PROXY_ {
		static const NodeType RESULT = t;
	};
	template <typename T>
	struct IS_BASE_TYPE_ {
		static const bool RESULT = std::is_same<T, typename BASE_TYPE_<T>::RESULT>::value;
	};

	// IS_POOLED Flag
	SET_POOLED(ENGINE, 						false);
	SET_POOLED(RENDERER, 					false);
	SET_POOLED(UPDATER, 					false);
	SET_POOLED(EMPTY, 						false);
	SET_POOLED(LOGIC, 						false);
	SET_POOLED(TRANSFORM, 					true);
	SET_POOLED(REGION, 						false);
	SET_POOLED(BOUNDING_BOX, 				true);
	SET_POOLED(STATIC_OBJECT_MANAGER, 		false);
	SET_POOLED(DYNAMIC_OBJECT_MANAGER, 		false);
	SET_POOLED(CAMERA, 						false);
	SET_POOLED(NANOGUI_SCREEN, 				false);
	SET_POOLED(SCENE_ROOT, 					false);
	SET_POOLED(SKYBOX, 						false);

	SET_POOLED(CONTENT_MANAGER, 			false);
	SET_POOLED(GEOMETRY, 					false);
	SET_POOLED(MATERIAL, 					false);
	SET_POOLED(SHADER, 						false);
	SET_POOLED(TEXTURE, 					false);
	SET_POOLED(STATIC_MESH, 				false);
	SET_POOLED(HALF_EDGE_GEOMETRY, 			false);
	SET_POOLED(SAMPLER, 					false);

	// The IS_RENDERABLE FLAG
	SET_RENDERABLE(ENGINE, 					false);
	SET_RENDERABLE(RENDERER, 				false);
	SET_RENDERABLE(UPDATER, 				false);
	SET_RENDERABLE(EMPTY, 					true);
	SET_RENDERABLE(LOGIC, 					true);
	SET_RENDERABLE(TRANSFORM, 				true);
	SET_RENDERABLE(REGION, 					true);
	SET_RENDERABLE(BOUNDING_BOX, 			true);
	SET_RENDERABLE(STATIC_OBJECT_MANAGER, 	true);
	SET_RENDERABLE(DYNAMIC_OBJECT_MANAGER, 	true);
	SET_RENDERABLE(CAMERA, 					true);
	SET_RENDERABLE(NANOGUI_SCREEN, 			true);
	SET_RENDERABLE(SCENE_ROOT, 				true);
	SET_RENDERABLE(SKYBOX, 					true);

	SET_RENDERABLE(HALF_EDGE_GEOMETRY, 		false);
	SET_RENDERABLE(CONTENT_MANAGER, 		false);
	SET_RENDERABLE(GEOMETRY, 				false);
	SET_RENDERABLE(MATERIAL, 				false);
	SET_RENDERABLE(SHADER, 					false);
	SET_RENDERABLE(TEXTURE, 				false);
	SET_RENDERABLE(STATIC_MESH, 			true);
	SET_RENDERABLE(SAMPLER, 				false);

	// The content flag
	SET_CONTENT(HALF_EDGE_GEOMETRY, 		true);
	SET_CONTENT(CONTENT_MANAGER, 			false);
	SET_CONTENT(GEOMETRY, 					true);
	SET_CONTENT(MATERIAL, 					true);
	SET_CONTENT(SHADER, 					true);
	SET_CONTENT(TEXTURE, 					true);
	SET_CONTENT(STATIC_MESH, 				true);
	SET_CONTENT(SAMPLER, 					true);

	template <typename T, 
		bool type_check=std::is_same<T, typename BASE_TYPE_<T>::RESULT>::value>
	struct ref;

	typedef IDisposable* (*DisposableConverter)(ref<void>&);
	typedef IUpdatable* (*UpdatableConverter)(ref<void>&);
	typedef IInitializable* (*InitializableConverter)(ref<void>&);
	
	struct NodeData;
	template <class T>
	T* getInterface(NodeData& d);

	template <>
	inline IDisposable* getInterface<IDisposable>(NodeData& d);
	template <>
	inline IUpdatable* getInterface<IUpdatable>(NodeData& d);
	template <>
	inline IInitializable* getInterface<IInitializable>(NodeData& d);

	
	// A static class used to obtain template metaprogramming values during runtime.
	class NodeMetadata {
	public:
		typedef void (*disposer)(ref<void>&);

	private:
		static bool pooled[(uint32_t)NodeType::END];
		static bool renderable[(uint32_t)NodeType::END];
		static bool content[(uint32_t)NodeType::END];
		static NodeType proxyToPrototype[(uint32_t)NodeType::END];
		static NodeType prototypeToProxy[(uint32_t)NodeType::END];
		static DisposableConverter disposableConv[(uint32_t)NodeType::END];
		static UpdatableConverter updatableConv[(uint32_t)NodeType::END];
		static InitializableConverter initializableConv[(uint32_t)NodeType::END];

		static void init();

	public:
		
		// Returns whether or not the given node type is a pooled type. Pooled types
		// are allocated from an object pool and must be referenced through a Pool handle,
		// as the memory for the object may be moved during runtime.
		// t: The type to query.
		// returns: Whether or not t is a pooled type.
		static inline bool isPooled(NodeType t) { 
			return pooled[(uint32_t)t]; 
		}

		
		// Returns whether or not the given node type is a scene child. Scene children are
		// the only nodes which may be traversed by the renderer. Anything which is not a scene
		// child is ignored by the renderer. Note that some node types may be prototypes for instances
		// which are scene children, even if they themselves are not.
		// t: The type to query.
		// returns: Whether or not t is a scene child type.
		static inline bool isRenderable(NodeType t) { 
			return renderable[(uint32_t)t]; 
		}
		
		
		// Returns whether or not this node type is owned by the ContentManager.
		// t: The type to query.
		// returns: Whether or not t is a content type.
		static inline bool isContent(NodeType t) {
			return content[(uint32_t)t];
		}

		
		// Gets the proxy type of a prototype type. Some types have instance versions, i.e., MATERIAL
		// and MATERIAL_PROXY, that may have different properties. For example, MATERIAL_INSTANCE is
		// a scene child, but MATERIAL is not. This means that the renderer will ignore a MATERIAL node,
		// but not a MATERIAL_PROXY node, a distinction which is used for garbage collection by the
		// ContentManager. You must register proxy types with the DEF_PROXY macro. if an instance
		// type is not registered, the function will return t.
		// t: The type to query.
		// returns: The instance type of t.
		static inline NodeType getProxyType(NodeType t) { 
			return prototypeToProxy[(uint32_t)t]; 
		}

		
		// The opposite of getInstanceType. Given a proxy type, i.e., MATERIAL_PROXY, this returns
		// the prototype type, i.e., MATERIAL. You must register instance types with the DEF_PROXY macro. 
		// if an instance type is not registered, the function will return t.
		// t: The type to query.
		// returns: The prototype type of t. 
		static inline NodeType getPrototypeType(NodeType t) { 
			return proxyToPrototype[(uint32_t)t]; 
		}

		friend class Engine;
		friend IDisposable* getInterface<IDisposable>(NodeData& d);
		friend IUpdatable* getInterface<IUpdatable>(NodeData& d);
		friend IInitializable* getInterface<IInitializable>(NodeData& d);
	};

	template <>
	struct ref<void, true> {
	public:
		union voidrefunion {
			PoolHandle<void> mHandle;
			void* mPtr;
			inline voidrefunion() { }
			inline ~voidrefunion() { }
		} p;

		inline ref() {
		}
		inline ref(void* ptr) {
			p.mPtr = ptr;
		}
		inline ref(PoolHandle<void>& h) {
			p.mHandle = h;
		}
		template <typename T>
		inline ref<T> reinterpret();
		template <typename T>
		inline T* reinterpretGet();
		template <typename T>
		inline ref(const ref<T>& other);
		template <typename T>
		inline T* getAs();
	};

	template <typename T, bool pooled>
	struct REF_POOL_GATE_;

	template <typename T>
	struct REF_POOL_GATE_<T, true> {
		PoolHandle<T> mHandle;
		inline T* get() const { return mHandle.get(); }
		inline void from(T* ptr) {
			assert(true);
		}
		inline void from(const PoolHandle<T>& newH) {
			mHandle = newH;
		}
		inline void from(const ref<void>& r) {
			mHandle = r.p.mHandle.reinterpret<T>();
		}
		inline void to(ref<void>& r) const {
			r.p.mHandle = PoolHandle<void>(mHandle);
		}
		inline T* getPtr() const {
			assert(true);
			return nullptr;
		}
		inline PoolHandle<T> getPoolHandle() const {
			return mHandle;
		}
		inline bool compare(const REF_POOL_GATE_<T, true>& other) {
			return (mHandle.mPoolPtr == other.mHandle.mPoolPtr &&
				mHandle.mOffset == other.mHandle.mOffset);
		}
		inline static T* get(const ref<void>& obj) {
			return ((Pool<T>*)obj.p.mHandle.mPoolPtr)->at(obj.p.mHandle.mOffset);
		}
	};

	template <typename T>
	struct REF_POOL_GATE_<T, false> {
		T* mPtr;
		inline T* get() const { return mPtr; }
		inline void from(T* newPtr) {
			mPtr = newPtr;
		}
		inline void from(const PoolHandle<T>& h) {
			assert(true);
		}
		inline void from(const ref<void>& r) {
			mPtr = (T*)r.p.mPtr;
		}
		inline void to(ref<void>& r) const {
			r.p.mPtr = mPtr;
		}
		inline T* getPtr() const {
			return mPtr;
		}
		inline PoolHandle<T> getPoolHandle() const {
			assert(true);
			return PoolHandle<T>();
		}
		inline bool compare(const REF_POOL_GATE_<T, false>& other) {
			return (mPtr == other.mPtr);
		}
		inline static T* get(const ref<void>& obj) {
			return (T*)obj.p.mPtr;
		}
	};

	
	// A pointer substitute that allows one to mix pooled and unpooled types. Note: this should only be used with base types!
	// T: The type that this is a reference to.
	template <typename T>
	struct ref<T, true> {
	private:
		REF_POOL_GATE_<T, IS_POOLED_<NODE_TYPE_<typename 
			BASE_TYPE_<T>::RESULT>::RESULT>::RESULT> mPoolGate;

	public:
		inline T* get() const { return mPoolGate.get(); }
		inline T* operator->() const { return mPoolGate.get(); }
		inline bool operator==(const ref<T>& other) { return mPoolGate.compare(other.mPoolGate); }

		inline PoolHandle<T> getPoolHandle() const {
			return mPoolGate.getPoolHandle();
		}

		inline ref<void> base() const {
			ref<void> r;
			mPoolGate.to(r);
			return r;
		}

		inline ref() { }

		inline ref(T* ptr) {
			mPoolGate.from(ptr);
		}

		inline ref(const PoolHandle<T>& h) {
			mPoolGate.from(h);
		}

		template <typename T2>
		inline T2* getAs() const {
			T* ptr = mPoolGate.get();
			return dynamic_cast<T2*>(ptr);
		}

		friend struct ref<void>;
	};

	template <typename T>
	inline ref<T> ref<void, true>::reinterpret() {
		ref<T> a;
		a.mPoolGate.from(*this);
		return a;
	}

	template <typename T>
	inline T* ref<void, true>::getAs() {
		return dynamic_cast<T*>(REF_POOL_GATE_<T,
			IS_POOLED_<NODE_ENUM(typename BASE_TYPE_<T>::RESULT)>::RESULT>::get(*this));
	}

	template <typename T>
	inline T* ref<void, true>::reinterpretGet() {
		return REF_POOL_GATE_<T, IS_POOLED_<NODE_ENUM(T)>::RESULT>::get(*this);
	}

	template <typename T>
	inline ref<void, true>::ref(const ref<T>& other) {
		other.mPoolGate.to(*this);
	}

	struct NodeData {
		NodeType type;
		ref<void> owner;
	};

	template <typename T>
	inline T* getOwner(NodeData& n) {
		assert(NODE_ENUM(T) == n.type || PROTOTYPE_TO_PROXY_<NODE_ENUM(T)>::RESULT == n.type);
		return n.owner.reinterpretGet<T>();
	}

	template <>
	inline IDisposable* getInterface<IDisposable>(NodeData& d) {
		return (*NodeMetadata::disposableConv[(uint32_t)d.type])(d.owner);
	}
	template <>
	inline IUpdatable* getInterface<IUpdatable>(NodeData& d) {
		return (*NodeMetadata::updatableConv[(uint32_t)d.type])(d.owner);
	}
	template <>
	inline IInitializable* getInterface<IInitializable>(NodeData& d) {
		return (*NodeMetadata::initializableConv[(uint32_t)d.type])(d.owner);
	}

	typedef DigraphDataView<NodeData> NodeDataView;
	typedef DigraphVertexLookupView<NodeHandle> NodeHandleLookupView;
	typedef DigraphVertexLookupView<std::string> NodeNameLookupView;

	
	/// A directed graph containing all nodes of the current engine instance.
	
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
		inline NodeData* desc(const Node& v) {
			return &(mDescs[v]);
		}
		inline NodeData* desc(const std::string& s) {
			return &(mDescs[mNames[s]]);
		}
		inline NodeData* desc(const NodeHandle h) {
			return &(mDescs[mHandles[h]]);
		}
		inline NodeHandleLookupView handles() const {
			return mHandles;
		}
		inline NodeNameLookupView names() const {
			return mNames;
		}
		inline Node operator[](const NodeHandle handle) {
			return mHandles[handle];
		}
		inline Node operator[](const std::string& name) {
			return mNames[name];
		}
		inline Node find(const NodeHandle handle) {
			return mHandles[handle];
		}
		inline Node find(const std::string& name) {
			return mNames[name];
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeType type) {
			assert(type != NodeType::END);
			NodeData data;
			data.type = type;
			data.owner = ref<void>(owner);
			auto v = createVertex();
			mDescs[v] = data;
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeType type, Node& parent) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeType type, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, type);
			createEdge(mNames[parentName], v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, Node& parent) {
			auto v = addNode<OwnerType>(owner, NODE_ENUM(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, NODE_ENUM(OwnerType));
			Node u = mHandles[parentHandle];
			createEdge(u, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_ENUM(OwnerType));
			createEdge(mNames[parentName], v);
			return v;
		}
		inline Node addNode(ref<void> owner, NodeType type) {
			NodeData data;
			data.type = type;
			data.owner = owner;
			auto v = createVertex();
			mDescs[v] = data;
			return v;
		}
		inline Node addNode(ref<void> owner, NodeType type, Node& parent) {
			auto v = addNode(owner, type);
			createEdge(parent, v);
			return v;
		}
		inline Node addNode(ref<void> owner, NodeType type, NodeHandle parentHandle) {
			auto v = addNode(owner, type);
			auto p = mHandles[parentHandle];
			createEdge(p, v);
			return v;
		}
		inline Node addNode(ref<void> owner, NodeType type, const std::string& parentName) {
			auto v = addNode(owner, type);
			auto p = mNames[parentName];
			createEdge(p, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(ref<void> owner, Node& parent) {
			auto v = addNode(owner, NODE_ENUM(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(ref<void> owner, NodeHandle parentHandle) {
			auto v = addNode(owner, NODE_ENUM(OwnerType));
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(ref<void> owner, const std::string& parentName) {
			auto v = addNode(owner, NODE_ENUM(OwnerType));
			createEdge(mNames[parentName], v);
			return v;
		}

		
		// Make an proxy of the node type. To automatically add the prototype 
		// as a child of the instance, use makeContentInstance instead.
		// base: The prototype of the proxy.
		// returns: A proxy node of the prototype. 
		Node makeProxy(const Node& base) {
			auto& desc = mDescs[base];
			NodeType instanceType = NodeMetadata::getProxyType(desc.type);
			assert(instanceType != desc.type);
			return addNode(ref<void>(nullptr), instanceType);
		}

		
		// Same as makeProxy, except that the prototype is added as a child to the
		// proxy node. This is useful for content types, so that the prototype isn't
		// collected by the ContentManager garbage collector while it is in use through
		// an instance node.
		// base: The prototype of the proxy.
		// returns: A proxy node of the prototype. 
		Node makeContentProxy(const Node& base) {
			Node v = makeProxy(base);
			createEdge(v, base);
			return v;
		}

		// Add a node to the graph with the specified owner.
		// T: The type of the owner.
		// owner: The owner of the new node.
		// returns: A new node in the graph. 
		template <typename T> Node addNode(T* owner) {
			return addNode(ref<void>(owner), NODE_ENUM(T));
		}

		
		// Add a node to the graph with the specified owner.
		// T: The type of the owner.
		// owner: The owner of the new node.
		// returns: A new node in the graph. 
		template <typename T> Node addNode(const ref<T>& owner) {
			return addNode(owner.base(), NODE_ENUM(T));
		}

		// Issues a handle for the specified node. The location of the node may be moved
		// in memory during runtime, so a handle assures consistent access to the node
		// even if it is moved. You can use a handle to aquire a Node object via the
		// [] operator.
		// vertex: The vertex to issue a handle for.
		// returns: A handle for the vertex. 
		inline NodeHandle issueHandle(const Node& vertex) {
			NodeHandle h = ++mLargestHandle;
			mHandles.set(vertex, h);
			return h;
		}
		
		// Recalls a handle that has been issued by the graph, specifying that it is no
		// longer in use. 
		// handle: The handle to recall.
		inline void recallHandle(const NodeHandle handle) {
			mHandles.clear(handle);
		}
		
		// Sets the name of a vertex. Vertices assigned a name can be accessed via 
		// the [] operator.
		// vertex: The vertex to assign a name to.
		// name: The name to assign.
		inline void setName(const Node& vertex, const std::string& name) {
			mNames.set(vertex, name);
		}
		
		// Specifies to the graph that the given name is no longer in use.
		// name: The name to assign.
		inline void recallName(const std::string& name) {
			mNames.clear(name);
		}

		NodeGraph() : mLargestHandle(HANDLE_INVALID) {
			mDescs = createVertexData<NodeData>("desc");
			mHandles = createVertexLookup<NodeHandle>("handle");
			mNames = createVertexLookup<std::string>("name");
		}
	};

	std::string nodeTypeString(NodeType t);
	void print(Node node);

	struct Transform {
		glm::vec3 mTranslation;
		glm::vec3 mScale;
		glm::quat mRotation;
		glm::fmat4 mCache;

		inline glm::fmat4 apply(const glm::fmat4& mat) const {
			auto ret = glm::scale(mat, mScale);
			ret = glm::translate(ret, mTranslation);
			return ret * glm::mat4_cast(mRotation);
		}

		inline void cache(const glm::fmat4& parent) {
			mCache = apply(parent);
		}

		static Node makeIdentity(Node parent, Node scene, ref<Transform>* pTransform = nullptr);
		static Node makeIdentity(Node parent, ref<Scene> scene, ref<Transform>* pTransform = nullptr);
		static Node makeTranslation(const glm::vec3& translation, Node parent, Node scene, ref<Transform>* pTransform = nullptr);
		static Node makeTranslation(const glm::vec3& translation, Node parent, ref<Scene> scene, ref<Transform>* pTransform = nullptr);
		static Node makeRotation(const glm::quat& rotate, Node parent, Node scene, ref<Transform>* pTransform = nullptr);
		static Node makeRotation(const glm::quat& rotate, Node parent, ref<Scene> scene, ref<Transform>* pTransform = nullptr);
	};

	enum class ErrorCode {
		SUCCESS,
		FAIL_GLFW_INIT,
		FAIL_GLFW_WINDOW_INIT,
		FAIL_LOAD_CONFIG
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

	SET_NODE_ENUM(Engine, ENGINE);
	SET_NODE_ENUM(ContentManager, CONTENT_MANAGER);
	SET_NODE_ENUM(BoundingBox, BOUNDING_BOX);
	SET_NODE_ENUM(char, EMPTY);
	SET_NODE_ENUM(Transform, TRANSFORM);
	SET_NODE_ENUM(IRenderer, RENDERER);

	template <typename T>
	ref<T> duplicateRef(const ref<T>& a);
	template <typename T>
	Node duplicateToNode(const ref<T>& a);
	template <typename T>
	Node duplicate(const Node& a);

	#ifdef _DEBUG
	#define GL_ASSERT checkGLError();
	#else
	#define GL_ASSERT
	#endif

	void printError(unsigned int err);
	void checkGLError();

	// Any class that derives from ILogic is meant to be a nonvisible node in
	// the game world that controls or modifies other nodes.
	class ILogic : public IDisposable, public IUpdatable, public IInitializable {
	};
	SET_NODE_ENUM(ILogic, LOGIC);

	// Initializes all descendants of the node that are scene children.
	// node: The ancestor node to start at.
	void init(Node node);
	
	/// Destroys all ancestors of a node in the scene graph, self included.
	/// bIgnoreContent: Whether or to ignore nodes managed by the content manager.
	void prune(Node start, bool bIgnoreContent = true);

	// Destroys all ancestors of a node with the given name.
	// name: Node of the node to prune.
	// bIgnoreContent: Whether to ignore nodes managed by the content manager.
	void prune(const std::string& name, bool bIgnoreContent = true);

	// Destroys all ancestors of a node with the given handle.
	// handle: Handle of the node to prune.
	// bIgnoreContent: Whether to ignore nodes managed by the content manager.
	void prune(NodeHandle handle, bool bIgnoreContent = true);
}