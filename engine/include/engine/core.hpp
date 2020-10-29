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
#include <engine/geobase.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

#define HANDLE_INVALID 0

#define SET_CONTENT(Type, bIsContent) template <> struct IS_CONTENT_<NodeType::Type> \
	{ static constexpr bool RESULT = bIsContent; }

#define SET_RENDERABLE(Type, bIsChild) template <> struct IS_RENDERABLE_<NodeType::Type>	\
	{ static constexpr bool RESULT = bIsChild; }

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

#define MORPHEUS_STRINGIFY(a) #a

struct GLFWwindow;

namespace Morpheus {

	class ContentManager;
	class Engine;
	struct Transform;
	class IContentFactory;
	class Updater;
	class Scene;
	class GuiBase;
	class IRenderer;
	class Shader;
	class Material;
	class StaticMesh;
	class Geometry;
	class ILogic;
	class Sampler;
	class HalfEdgeGeometry;
	class Texture;
	class Camera;
	class TransformNode;
	class Skybox;
	class Framebuffer;

	typedef DigraphVertex Node;
	
	/// Any class that should be a child of the updater and updated every frame.
	class IUpdatable {
	public:
		virtual void setEnabled(bool) = 0;
		virtual bool isEnabled() const = 0;
		virtual void update(double dt) = 0;
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

	enum class NodeType : uint32_t {
		START,

		ENGINE,
		RENDERER,
		UPDATER,
		
		// Misc.
		LAMBERT_SH_COMPUTE_KERNEL,
		LAMBERT_COMPUTE_KERNEL,
		GGX_COMPUTE_KERNEL,
		GGX_COMPUTE_KERNEL_OLD,
		COOK_TORRANCE_LUT_COMPUTE_KERNEL,
		SPRITE_BATCH,
		LINE_BATCH,
		TRIANGLE_BATCH,
		POINT_BATCH,
		DEBUG_BATCH,

		CUSTOM_NO_RENDER_NOT_CONTENT,
		CUSTOM_KERNEL,

		// All nodes that are found inside of a scene
		SCENE_BEGIN,
		CAMERA,
		SCENE_ROOT,
		ACCELERATOR,
		ENTITY,
		TRANSFORM,
		REGION,
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
		FRAMEBUFFER,

		END
	};

	template <typename T>
	struct BASE_TYPE_ {
		typedef T RESULT;
	};
	template <typename T>
	struct NODE_TYPE_;

	template <NodeType>
	struct OWNER_TYPE_ {
		typedef void RESULT;
	};

	template <NodeType>
	struct IS_RENDERABLE_ {
		static const bool RESULT = false;
	};
	template <NodeType>
	struct IS_CONTENT_ {
		static const bool RESULT = false;
	};
	template <typename T>
	struct IS_BASE_TYPE_ {
		static const bool RESULT = std::is_same<T, typename BASE_TYPE_<T>::RESULT>::value;
	};

	// The IS_RENDERABLE FLAG
	SET_RENDERABLE(ENGINE, 								false);
	SET_RENDERABLE(RENDERER, 							false);
	SET_RENDERABLE(UPDATER, 							false);

	SET_RENDERABLE(LAMBERT_SH_COMPUTE_KERNEL,			false);
	SET_RENDERABLE(LAMBERT_COMPUTE_KERNEL,				false);
	SET_RENDERABLE(GGX_COMPUTE_KERNEL_OLD,				false);
	SET_RENDERABLE(GGX_COMPUTE_KERNEL,					false);
	SET_RENDERABLE(COOK_TORRANCE_LUT_COMPUTE_KERNEL, 	false);
	SET_RENDERABLE(SPRITE_BATCH,						false);
	SET_RENDERABLE(LINE_BATCH,							false);
	SET_RENDERABLE(TRIANGLE_BATCH,						false);
	SET_RENDERABLE(POINT_BATCH,							false);
	SET_RENDERABLE(DEBUG_BATCH,							false);
	SET_RENDERABLE(CUSTOM_NO_RENDER_NOT_CONTENT,		false);
	SET_RENDERABLE(CUSTOM_KERNEL,						false);

	SET_RENDERABLE(ENTITY, 								true);
	SET_RENDERABLE(TRANSFORM, 							true);
	SET_RENDERABLE(REGION, 								true);
	SET_RENDERABLE(STATIC_OBJECT_MANAGER, 				true);
	SET_RENDERABLE(DYNAMIC_OBJECT_MANAGER, 				true);
	SET_RENDERABLE(CAMERA, 								true);
	SET_RENDERABLE(NANOGUI_SCREEN, 						true);
	SET_RENDERABLE(SCENE_ROOT, 							true);
	SET_RENDERABLE(ACCELERATOR,							true);
	SET_RENDERABLE(SKYBOX, 								true);

	SET_RENDERABLE(HALF_EDGE_GEOMETRY, 					false);
	SET_RENDERABLE(CONTENT_MANAGER, 					false);
	SET_RENDERABLE(GEOMETRY, 							false);
	SET_RENDERABLE(MATERIAL, 							false);
	SET_RENDERABLE(SHADER, 								false);
	SET_RENDERABLE(TEXTURE, 							false);
	SET_RENDERABLE(STATIC_MESH, 						true);
	SET_RENDERABLE(SAMPLER, 							false);
	SET_RENDERABLE(FRAMEBUFFER,							false);

	// The content flag
	SET_CONTENT(LAMBERT_SH_COMPUTE_KERNEL, 				false);
	SET_CONTENT(LAMBERT_COMPUTE_KERNEL,					false);
	SET_CONTENT(GGX_COMPUTE_KERNEL,						false);
	SET_CONTENT(GGX_COMPUTE_KERNEL_OLD,					false);
	SET_CONTENT(COOK_TORRANCE_LUT_COMPUTE_KERNEL, 		false);
	SET_CONTENT(SPRITE_BATCH,							false);
	SET_CONTENT(LINE_BATCH,								false);
	SET_CONTENT(POINT_BATCH,							false);
	SET_CONTENT(DEBUG_BATCH,							false);
	SET_CONTENT(TRIANGLE_BATCH,							false);
	SET_CONTENT(CUSTOM_NO_RENDER_NOT_CONTENT,			false);
	SET_CONTENT(CUSTOM_KERNEL,							false);
	SET_CONTENT(ACCELERATOR,							false);

	SET_CONTENT(HALF_EDGE_GEOMETRY, 					true);
	SET_CONTENT(CONTENT_MANAGER, 						false);
	SET_CONTENT(GEOMETRY, 								true);
	SET_CONTENT(MATERIAL, 								true);
	SET_CONTENT(SHADER, 								true);
	SET_CONTENT(TEXTURE, 								true);
	SET_CONTENT(STATIC_MESH, 							true);
	SET_CONTENT(SAMPLER, 								true);
	SET_CONTENT(FRAMEBUFFER,							true);

	struct ByteColor {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	// A static class used to obtain template metaprogramming values during runtime.
	class NodeMetadata {
	private:
		static bool renderable[(uint32_t)NodeType::END];
		static bool content[(uint32_t)NodeType::END];

		static void init();

	public:
		
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

		friend class Engine;
	};

	class INodeOwner;
	class NodeGraph;

	struct NodeOwnerIteratorF {
	private:
		NodeGraph* mGraph;
		DigraphVertexIteratorF mInternalIt;

	public:
		inline NodeOwnerIteratorF(NodeGraph* graph, const DigraphVertexIteratorF& it) :
			mGraph(graph), mInternalIt(it) {
		}
		inline INodeOwner* operator()();
		inline bool valid() const {
			return mInternalIt.valid();
		}
		inline void next() {
			mInternalIt.next();
		}
	};

	struct NodeOwnerIteratorB {
	private:
		NodeGraph* mGraph;
		DigraphVertexIteratorB mInternalIt;

	public:
		inline NodeOwnerIteratorB(NodeGraph* graph, const DigraphVertexIteratorB& it) :
			mGraph(graph), mInternalIt(it) {
		}
		inline INodeOwner* operator()();
		inline bool valid() const {
			return mInternalIt.valid();
		}
		inline void next() {
			mInternalIt.next();
		}
	};

	struct RaycastInfo {
	public:
		INodeOwner* mNode;
		float mDistance;
		glm::vec3 mLocation;
	};

	class INodeOwner {
	private:
		int mNodeId;
		NodeGraph* mGraph;
		NodeType mNodeType;

	public:
		inline INodeOwner(NodeType type) : mNodeType(type), mNodeId(-1), mGraph(nullptr) {
		}

		inline NodeType getType() const {
			return mNodeType;
		}

		inline const NodeGraph* graph() const {
			return mGraph;
		}

		inline NodeGraph* graph() {
			return mGraph;
		}

		inline Node node();

		inline bool hasNode() const {
			return mNodeId != -1;
		}
		
		inline NodeOwnerIteratorF children();
		inline NodeOwnerIteratorB parents();
		virtual void addChild(INodeOwner* child);
		virtual void addParent(INodeOwner* parent);
		inline uint32_t childCount();
		inline uint32_t parentCount();
		inline uint32_t outDegree();
		inline uint32_t inDegree();
		inline INodeOwner* getChild(const uint32_t i);
		inline INodeOwner* getParent(const uint32_t i);
		
		inline bool isContent() const {
			return NodeMetadata::isContent(mNodeType);
		}

		inline bool isRenderable() const {
			return NodeMetadata::isRenderable(mNodeType);
		}

		virtual void removeChild(INodeOwner* owner);
		virtual void removeParent(INodeOwner* owner);

		template <typename T>
		inline T& data(const std::string& s);

		virtual bool isUpdatable() const { return false; }
		virtual void setEnabled(bool v) { }
		virtual bool isEnabled() const { return true; }
		virtual void update(double dt) { }
		virtual void init() { }

		// By default, we just check all children
		virtual bool raycast(const Ray& ray, RaycastInfo* result) {
			result->mDistance = std::numeric_limits<float>::infinity();
			bool bHit = false;

			RaycastInfo info;
			for (auto it = children(); it.valid(); it.next()) {
				auto child = it();
				if (child->raycast(ray, &info) && info.mDistance < result->mDistance) {
					*result = info;
					bHit = true;
				}
			}

			return bHit;
		}

		virtual glm::vec3 computeCenter() const {
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		virtual BoundingBox computeBoundingBox() const {
			return BoundingBox::empty();
		}

		virtual bool hasBoundingBox() const {
			return false;
		}

		virtual Shader* toShader();
		virtual StaticMesh* toStaticMesh();
		virtual Sampler* toSampler();
		virtual Geometry* toGeometry();
		virtual HalfEdgeGeometry* toHalfEdgeGeometry();
		virtual Texture* toTexture();
		virtual Material* toMaterial();
		virtual Scene* toScene();
		virtual Camera* toCamera();
		virtual GuiBase* toGui();
		virtual TransformNode* toTransform();
		virtual Skybox* toSkybox();
		virtual Framebuffer* toFramebuffer();

		virtual ~INodeOwner() {}

		friend class NodeGraph;
	};

	template <typename T>
	T* convert(INodeOwner* o);

	template <>
	inline Shader* convert<Shader>(INodeOwner* o) {
		return o->toShader();
	}

	template <>
	inline StaticMesh* convert<StaticMesh>(INodeOwner* o) {
		return o->toStaticMesh();
	}

	template <>
	inline Sampler* convert<Sampler>(INodeOwner* o) {
		return o->toSampler();
	}

	template <>
	inline Geometry* convert<Geometry>(INodeOwner* o) {
		return o->toGeometry();
	}

	template <>
	inline HalfEdgeGeometry* convert<HalfEdgeGeometry>(INodeOwner* o) {
		return o->toHalfEdgeGeometry();
	}

	template <>
	inline Texture* convert<Texture>(INodeOwner* o) {
		return o->toTexture();
	}

	template <>
	inline Material* convert<Material>(INodeOwner* o) {
		return o->toMaterial();
	}

	template <>
	inline Scene* convert<Scene>(INodeOwner* o) {
		return o->toScene();
	}

	template <>
	inline Camera* convert<Camera>(INodeOwner* o) {
		return o->toCamera();
	}

	template <>
	inline GuiBase* convert<GuiBase>(INodeOwner* o) {
		return o->toGui();
	}

	template <>
	inline TransformNode* convert<TransformNode>(INodeOwner* o) {
		return o->toTransform();
	}

	typedef DigraphDataView<INodeOwner*> OwnerDataView;
	typedef DigraphVertexLookupView<std::string> NodeNameLookupView;
	
	/// A directed graph containing all nodes of the current engine instance.
	class NodeGraph : public Digraph {
	private:
		// Descriptions of the types and owners of each node
		DigraphDataView<INodeOwner*> mOwners;
		DigraphVertexLookupView<std::string> mNames;

	protected:
	 	void applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize) override;

	public:
		inline void setOwner(const Node& v, INodeOwner* owner) {
			mOwners[v] = owner;
			owner->mGraph = this;
			owner->mNodeId = v.id();
		}

		inline OwnerDataView& owners() { 
			return mOwners;
		}
		inline INodeOwner* owner(const Node& v) {
			return mOwners[v];
		}
		inline INodeOwner* find(const std::string& s) {
			return mOwners[mNames[s]];
		}
		inline NodeNameLookupView names() const {
			return mNames;
		}
		inline bool tryFind(const std::string& name, INodeOwner** out) {
			Node n;
			if (mNames.tryFind(name, &n)) {
				*out = mOwners[n];
				return true;
			}
			else {
				*out = nullptr;
				return false;
			}
		}
		inline void createNode(INodeOwner* owner) {
			assert(owner->mNodeType != NodeType::END);
			auto v = createVertex();
			owner->mGraph = this;
			owner->mNodeId = v.id();
			mOwners[v] = owner;
		}
		inline void createNode(INodeOwner* owner, INodeOwner* parent) {
			createNode(owner);
			parent->addChild(owner);
		}
		inline void createNode(INodeOwner* owner, const std::string& parentName) {
			INodeOwner* parent = find(parentName);
			createNode(owner, parent);
		}
		// Sets the name of a vertex. Vertices assigned a name can be accessed via 
		// the [] operator.
		// vertex: The vertex to assign a name to.
		// name: The name to assign.
		inline void setName(const Node& vertex, const std::string& name) {
			mNames.set(vertex, name);
		}

		inline void setName(const INodeOwner* owner, const std::string& name) {
			mNames.set(Node(owner->mGraph, owner->mNodeId), name);
		}
		
		// Specifies to the graph that the given name is no longer in use.
		// name: The name to assign.
		inline void recallName(const std::string& name) {
			mNames.clear(name);
		}

		NodeGraph() {
			mOwners = createVertexData<INodeOwner*>("owner");
			mNames = createVertexLookup<std::string>("name");
		}
	};

	INodeOwner* NodeOwnerIteratorF::operator()() {
		return mGraph->owner(mInternalIt());
	}

	INodeOwner* NodeOwnerIteratorB::operator()() {
		return mGraph->owner(mInternalIt());
	}

	Node INodeOwner::node() {
		return Node(mGraph, mNodeId);
	}
	NodeOwnerIteratorF INodeOwner::children() {
		Node n(mGraph, mNodeId);
		return NodeOwnerIteratorF(mGraph, n.children());
	}
	NodeOwnerIteratorB INodeOwner::parents() {
		Node n(mGraph, mNodeId);
		return NodeOwnerIteratorB(mGraph, n.parents());
	}
	void INodeOwner::addChild(INodeOwner* child) {
		assert(child->hasNode());
		assert(hasNode());
		Node n(mGraph, mNodeId);
		Node n_child(mGraph, child->mNodeId);
		n.addChild(n_child);
	}
	void INodeOwner::addParent(INodeOwner* parent) {
		assert(parent->hasNode());
		assert(hasNode());
		Node n(mGraph, mNodeId);
		Node n_parent(mGraph, parent->mNodeId);
		n.addParent(n_parent);
	}
	uint32_t INodeOwner::childCount() {
		Node n(mGraph, mNodeId);
		return n.childCount();
	}
	uint32_t INodeOwner::parentCount() {
		Node n(mGraph, mNodeId);
		return n.parentCount();
	}
	uint32_t INodeOwner::outDegree() {
		Node n(mGraph, mNodeId);
		return n.outDegree();
	}
	uint32_t INodeOwner::inDegree() {
		Node n(mGraph, mNodeId);
		return n.inDegree();
	}
	INodeOwner* INodeOwner::getChild(const uint32_t i) {
		Node n(mGraph, mNodeId);
		Node n_child = n.getChild(i);
		return mGraph->owner(n_child);
	}
	INodeOwner* INodeOwner::getParent(const uint32_t i) {
		Node n(mGraph, mNodeId);
		Node n_parent = n.getParent(i);
		return mGraph->owner(n_parent);
	}
	void INodeOwner::removeChild(INodeOwner* child) {
		Node n(mGraph, mNodeId);
		int childId = child->node().id();
		for (auto it = n.outgoing(); it.valid(); it.next()) {
			auto e = it();
			if (e.head().id() == childId) {
				mGraph->deleteEdge(e);
				break;
			}
		}
	}
	void INodeOwner::removeParent(INodeOwner* parent) {
		Node n(mGraph, mNodeId);
		int parentId = parent->node().id();
		for (auto it = n.outgoing(); it.valid(); it.next()) {
			auto e = it();
			if (e.tail().id() == parentId) {
				mGraph->deleteEdge(e);
				break;
			}
		}
	}

	template <typename T>
	T& INodeOwner::data(const std::string& s) 
	{
		Node n(mGraph, mNodeId);
		return n.data<T>(s);
	}

	std::string nodeTypeString(NodeType t);
	void print(INodeOwner* start);

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

		static Transform makeIdentity();
		static Transform makeTranslation(const glm::vec3& translation);
		static Transform makeRotation(const glm::quat& rotate);

		inline static Transform makeTranslation(const float x, const float y, const float z) {
			return makeTranslation(glm::vec3(x, y, z));
		}
	};

	class TransformNode : public INodeOwner {
	public:
		Transform mTransform;

		TransformNode* toTransform() override;
		
		inline TransformNode() : INodeOwner(NodeType::TRANSFORM) {
			mTransform.mRotation = glm::identity<glm::quat>();
			mTransform.mTranslation = glm::zero<glm::vec3>();
			mTransform.mScale = glm::one<glm::vec3>();
		}
		inline TransformNode(const Transform& transform) : 
			mTransform(transform), 
			INodeOwner(NodeType::TRANSFORM) {
		}
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

	// The base class for all nodes containing logic
	class Entity : public INodeOwner {
	private:
		bool bEnabled;

	public:
		inline Entity() : bEnabled(true), INodeOwner(NodeType::ENTITY) {
		}

		bool isUpdatable() const override;
		bool isEnabled() const override;
		void setEnabled(const bool value) override;
	};

	SET_NODE_ENUM(Engine, ENGINE);
	SET_NODE_ENUM(ContentManager, CONTENT_MANAGER);
	SET_NODE_ENUM(TransformNode, TRANSFORM);
	SET_NODE_ENUM(IRenderer, RENDERER);
	SET_NODE_ENUM(Entity, ENTITY);

	#ifdef _DEBUG
	#define GL_ASSERT checkGLError();
	#else
	#define GL_ASSERT
	#endif

	void printError(unsigned int err);
	void checkGLError();

	// Initializes all descendants of the node that are scene children.
	// node: The ancestor node to start at.
	void init(INodeOwner* node);
	
	// Destroys all ancestors of a node in the scene graph, self included.
	// Content is handled separately by the content manager after the prune
	// is completed. If content has not been orphaned, it is not released,
	// as there may be multiple users.
	void prune(INodeOwner* node);

	// Destroys all ancestors of a node in the scene graph, self included.
	// Content is handled separately by the content manager after the prune
	// is completed. If content has not been orphaned, it is not released,
	// as there may be multiple users.
	void prune(const std::string& name);
}