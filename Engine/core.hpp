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

#include "json.hpp"
#include "digraph.hpp"
#include "pool.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
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

#define DEF_INSTANCE(InstanceType, NodeType_) template<> struct INSTANCE_TO_PROTOTYPE_<NodeType::InstanceType> \
	{ static const NodeType RESULT = NodeType::NodeType_; }; \
	template<> struct PROTOTYPE_TO_BASE_<NodeType::NodeType_> \
	{ static const NodeType RESULT = NodeType::InstanceType; } ;

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;

	typedef DigraphVertex Node;
	typedef uint32_t NodeHandle;
	
	class IDisposable {
	public:
		virtual void dispose() = 0;
	};

	class IUpdateable {
	public:
		virtual bool isEnabled() const = 0;
		virtual void setEnabled(const bool value) = 0;
		virtual void update(const double dt) = 0;
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
		STATIC_TRANSFORM,
		DYNAMIC_TRANSFORM,
		REGION,
		BOUNDING_BOX,
		STATIC_OBJECT_MANAGER,
		DYNAMIC_OBJECT_MANAGER,
		MATERIAL_INSTANCE,
		GEOMETRY_INSTANCE,
		NANOGUI_SCREEN,
		SCENE_END,

		// All nodes that are children of the content manager
		CONTENT_BEGIN,
		CONTENT_MANAGER,
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
	struct IS_SCENE_CHILD_ {
		static const bool RESULT = true;
	};
	template <NodeType>
	struct IS_DISPOSABLE_ {
		static const bool RESULT = true;
	};
	template <NodeType t>
	struct INSTANCE_TO_PROTOTYPE_ {
		static const NodeType RESULT = t;
	};
	template <NodeType t>
	struct PROTOTYPE_TO_BASE_ {
		static const NodeType RESULT = t;
	};

	// IS_POOLED Flag
	SET_POOLED(ENGINE, false);
	SET_POOLED(RENDERER, false);
	SET_POOLED(UPDATER, false);
	SET_POOLED(EMPTY, false);
	SET_POOLED(LOGIC, false);
	SET_POOLED(STATIC_TRANSFORM, true);
	SET_POOLED(DYNAMIC_TRANSFORM, true);
	SET_POOLED(REGION, false);
	SET_POOLED(BOUNDING_BOX, true);
	SET_POOLED(STATIC_OBJECT_MANAGER, false);
	SET_POOLED(DYNAMIC_OBJECT_MANAGER, false);
	SET_POOLED(CAMERA, false);
	SET_POOLED(MATERIAL_INSTANCE, false);
	SET_POOLED(NANOGUI_SCREEN, false);
	SET_POOLED(GEOMETRY_INSTANCE, false);

	SET_POOLED(CONTENT_MANAGER, false);
	SET_POOLED(GEOMETRY, false);
	SET_POOLED(MATERIAL, false);
	SET_POOLED(SHADER, false);
	SET_POOLED(TEXTURE_1D, false);
	SET_POOLED(TEXTURE_2D, false);
	SET_POOLED(TEXTURE_3D, false);
	SET_POOLED(CUBE_MAP, false);
	SET_POOLED(TEXTURE_2D_ARRAY, false);
	SET_POOLED(STATIC_MESH, false);

	// IS_SCENE_CHILD Flag
	SET_SCENE_CHILD(ENGINE, false);
	SET_SCENE_CHILD(RENDERER, false);
	SET_SCENE_CHILD(UPDATER, false);
	SET_SCENE_CHILD(EMPTY, true);
	SET_SCENE_CHILD(LOGIC, true);
	SET_SCENE_CHILD(STATIC_TRANSFORM, true);
	SET_SCENE_CHILD(DYNAMIC_TRANSFORM, true);
	SET_SCENE_CHILD(REGION, true);
	SET_SCENE_CHILD(BOUNDING_BOX, true);
	SET_SCENE_CHILD(STATIC_OBJECT_MANAGER, true);
	SET_SCENE_CHILD(DYNAMIC_OBJECT_MANAGER, true);
	SET_SCENE_CHILD(CAMERA, true);
	SET_SCENE_CHILD(NANOGUI_SCREEN, true);

	SET_SCENE_CHILD(CONTENT_MANAGER, false);
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
	SET_DISPOSABLE(UPDATER, false);
	SET_DISPOSABLE(EMPTY, false);
	SET_DISPOSABLE(LOGIC, true);
	SET_DISPOSABLE(STATIC_TRANSFORM, false);
	SET_DISPOSABLE(DYNAMIC_TRANSFORM, false);
	SET_DISPOSABLE(REGION, true);
	SET_DISPOSABLE(BOUNDING_BOX, false);
	SET_DISPOSABLE(STATIC_OBJECT_MANAGER, true);
	SET_DISPOSABLE(DYNAMIC_OBJECT_MANAGER, true);
	SET_DISPOSABLE(CAMERA, true);
	SET_DISPOSABLE(NANOGUI_SCREEN, true);

	SET_DISPOSABLE(CONTENT_MANAGER, true);
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

	/// <summary>
	/// A static class used to obtain template metaprogramming values during runtime.
	/// </summary>
	class NodeMetadata {
	public:
		typedef void (*disposer)(ref<void>&);

	private:
		static bool pooled[(uint32_t)NodeType::END];
		static bool sceneChild[(uint32_t)NodeType::END];
		static bool disposable[(uint32_t)NodeType::END];
		static NodeType instanceToPrototype[(uint32_t)NodeType::END];
		static NodeType prototypeToInstance[(uint32_t)NodeType::END];

		template <NodeType iType> 
		static void init_();
		template <> 
		static void init_<NodeType::END>();

		static void init();

	public:
		/// <summary>
		/// Returns whether or not the given node type is a pooled type. Pooled types
		/// are allocated from an object pool and must be referenced through a Pool handle,
		/// as the memory for the object may be moved during runtime.
		/// </summary>
		/// <param name="t">The type to query.</param>
		/// <returns>Whether or not t is a pooled type.</returns>
		static inline bool isPooled(NodeType t) { 
			return pooled[(uint32_t)t]; 
		}

		/// <summary>
		/// Returns whether or not the given node type is a scene child. Scene children are
		/// the only nodes which may be traversed by the renderer. Anything which is not a scene
		/// child is ignored by the renderer. Note that some node types may be prototypes for instances
		/// which are scene children, even if they themselves are not.
		/// </summary>
		/// <param name="t">The type to query.</param>
		/// <returns>Whether or not t is a scene child type.</returns>
		static inline bool isSceneChild(NodeType t) { 
			return sceneChild[(uint32_t)t]; 
		}
		
		/// <summary>
		/// Returns whether or not this node type implements the IDisposable interface. Disposable nodes
		/// typically contain memory that is not managed by the ContentManager or a Pool and must be
		/// deallocated separately.
		/// </summary>
		/// <param name="t">The type to query.</param>
		/// <returns>Whether or not t is a disposable type.</returns>
		static inline bool isDisposable(NodeType t) { 
			return disposable[(uint32_t)t]; 
		}

		/// <summary>
		/// Gets the instance type of a prototype type. Some types have instance versions, i.e., MATERIAL
		/// and MATERIAL_INSTANCE, that may have different properties. For example, MATERIAL_INSTANCE is
		/// a scene child, but MATERIAL is not. This means that the renderer will ignore a MATERIAL node,
		/// but not a MATERIAL_INSTANCE node, a distinction which is used for garbage collection by the
		/// ContentManager. You must register instance types with the DEF_INSTANCE macro. if an instance
		/// type is not registered, the function will return t.
		/// </summary>
		/// <param name="t">The type to query.</param>
		/// <returns>The instance type of t.</returns>
		static inline NodeType getInstanceType(NodeType t) { 
			return prototypeToInstance[(uint32_t)t]; 
		}

		/// <summary>
		/// The opposite of getInstanceType. Given an instance type, i.e., MATERIAL_INSTANCE, this returns
		/// the prototype type, i.e., MATERIAL. You must register instance types with the DEF_INSTANCE macro. 
		/// if an instance type is not registered, the function will return t.
		/// </summary>
		/// <param name="t">The type to query.</param>
		/// <returns>The prototype type of t.</returns>
		static inline NodeType getPrototypeType(NodeType t) { 
			return instanceToPrototype[(uint32_t)t]; 
		}

		friend class Engine;
	};

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
	struct REF_POOL_GATE_;

	template <typename T>
	struct REF_POOL_GATE_<T, true> {
		PoolHandle<T> mHandle;
		inline T* get() const { return mHandle.get(); }
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
		inline T* getPtr() const {
			assert(true);
			return nullptr;
		}
		inline PoolHandle<T> getPoolHandle() const {
			return mHandle;
		}
		inline static T* getAs(ref<void>& r) {
			return static_cast<T*>(r.p.mPtr);
		}
		inline bool compare(const REF_POOL_GATE_<T, true>& other) {
			return (mHandle.mPoolPtr == other.mHandle.mPoolPtr &&
				mHandle.mOffset == other.mHandle.mOffset);
		}
	};

	template <typename T>
	struct REF_POOL_GATE_<T, false> {
		T* mPtr;
		inline T* get() const { return mPtr; }
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
		inline T* getPtr() const {
			return mPtr;
		}
		inline PoolHandle<T> getPoolHandle() const {
			assert(true);
			return PoolHandle<T>();
		}
		inline static T* getAs(ref<void>& r) {
			return static_cast<T*>(r.p.mPtr);
		}
		inline bool compare(const REF_POOL_GATE_<T, false>& other) {
			return (mPtr == other.mPtr);
		}
	};

	template<typename T>
	inline T* Morpheus::ref<void>::getAs()
	{
		return REF_POOL_GATE_<T, IS_POOLED_<NODE_TYPE(T)>::RESULT>::getAs(*this);
	}

	template <typename T>
	struct ref {
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

		inline ref<void> asvoid() {
			ref<void> r;
			mPoolGate.to(r);
			return r;
		}

		inline ref() { }

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

	/// <summary>
	/// A directed graph containing all nodes of the current engine instance.
	/// </summary>
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
		inline NodeData& desc(const Node& v) {
			return mDescs[v];
		}
		inline NodeHandleLookupView handles() {
			return mHandles;
		}
		inline NodeNameLookupView names() {
			return mNames;
		}
		inline Node operator[](const NodeHandle handle) {
			return mHandles[handle];
		}
		inline Node operator[](const std::string& name) {
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
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, NodeHandle parentHandle) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
			Node u = mHandles[parentHandle];
			createEdge(u, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(OwnerType* owner, const std::string& parentName) {
			auto v = addNode<OwnerType>(owner, NODE_TYPE(OwnerType));
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
			auto v = addNode(owner, NODE_TYPE(OwnerType));
			createEdge(parent, v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(ref<void> owner, NodeHandle parentHandle) {
			auto v = addNode(owner, NODE_TYPE(OwnerType));
			createEdge(mHandles[parentHandle], v);
			return v;
		}
		template <typename OwnerType>
		inline Node addNode(ref<void> owner, const std::string& parentName) {
			auto v = addNode(owner, NODE_TYPE(OwnerType));
			createEdge(mNames[parentName], v);
			return v;
		}

		/// <summary>
		/// Make an instance of the node type. To automatically add the prototype 
		/// as a child of the instance, use makeContentInstance instead.
		/// </summary>
		/// <param name="base">The prototype of the instance.</param>
		/// <returns>An instance node of the prototype.</returns>
		Node makeInstance(const Node& base) {
			auto& desc = mDescs[base];
			NodeType instanceType = NodeMetadata::getInstanceType(desc.type);
			assert(instanceType != desc.type);
			return addNode(desc.owner, instanceType);
		}

		/// <summary>
		/// Same as makeInstance, except that the prototype is added as a child to the
		/// instance node. This is useful for content types, so that the prototype isn't
		/// collected by the ContentManager garbage collector while it is in use through
		/// an instance node.
		/// </summary>
		/// <param name="base">The prototype of the instance.</param>
		/// <returns>An instance node of the prototype.</returns>
		Node makeContentInstance(Node& base) {
			auto& desc = mDescs[base];
			NodeType instanceType = NodeMetadata::getInstanceType(desc.type);
			assert(instanceType != desc.type);
			return addNode(desc.owner, instanceType, base);
		}

		inline Node addNode(Engine* owner) {
			return addNode(owner, NodeType::ENGINE);
		}
		/// <summary>
		/// Issues a handle for the specified node. The location of the node may be moved
		/// in memory during runtime, so a handle assures consistent access to the node
		/// even if it is moved. You can use a handle to aquire a Node object via the
		/// [] operator.
		/// </summary>
		/// <param name="vertex">The vertex to issue a handle for.</param>
		/// <returns>A handle for the vertex.</returns>
		inline NodeHandle issueHandle(const Node& vertex) {
			NodeHandle h = ++mLargestHandle;
			mHandles.set(vertex, h);
			return h;
		}
		/// <summary>
		/// Recalls a handle that has been issued by the graph, specifying that it is no
		/// longer in use. 
		/// </summary>
		/// <param name="handle">The handle to recall.</param>
		inline void recallHandle(const NodeHandle handle) {
			mHandles.clear(handle);
		}
		/// <summary>
		/// Sets the name of a vertex. Vertices assigned a name can be accessed via 
		/// the [] operator.
		/// </summary>
		/// <param name="vertex">The vertex to assign a name to.</param>
		/// <param name="name">The name to assign.</param>
		inline void setName(const Node& vertex, const std::string& name) {
			mNames.set(vertex, name);
		}
		/// <summary>
		/// Specifies to the graph that the given name is no longer in use.
		/// </summary>
		/// <param name="name">The name to assign.</param>
		inline void recallName(const std::string& name) {
			mNames.clear(name);
		}

		NodeGraph() : mLargestHandle(HANDLE_INVALID) {
			mDescs = createVertexData<NodeData>("desc");
			mHandles = createVertexLookup<NodeHandle>("handle");
			mNames = createVertexLookup<std::string>("name");
		}
	};

	struct DynamicTransform {
		glm::vec3 mTranslation;
		glm::vec3 mScale;
		glm::quat mRotation;

		inline glm::mat4 apply(const glm::mat4& mat) const {
			auto ret = glm::scale(mat, mScale);
			ret = glm::translate(ret, mTranslation);
			return ret * glm::mat4_cast(mRotation);
		}

		inline glm::mat4 mat() const {
			return apply(glm::identity<glm::mat4>());
		}
	};

	struct StaticTransform {
		glm::mat4 mCache;

		inline void from(const DynamicTransform& transform) {
			mCache = transform.mat();
		}
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

	SET_NODE_TYPE(Engine, ENGINE);
	SET_NODE_TYPE(ContentManager, CONTENT_MANAGER);
	SET_NODE_TYPE(BoundingBox, BOUNDING_BOX);
	SET_NODE_TYPE(char, EMPTY);
	SET_NODE_TYPE(StaticTransform, STATIC_TRANSFORM);
	SET_NODE_TYPE(DynamicTransform, DYNAMIC_TRANSFORM);
	SET_NODE_TYPE(IRenderer, RENDERER);

	DEF_INSTANCE(GEOMETRY_INSTANCE, GEOMETRY);
	DEF_INSTANCE(MATERIAL_INSTANCE, MATERIAL);
}