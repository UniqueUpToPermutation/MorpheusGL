#include <glad/glad.h>

#include <engine/core.hpp>
#include <engine/engine.hpp>
#include <engine/content.hpp>
#include <engine/camera.hpp>
#include <engine/shader.hpp>
#include <engine/sampler.hpp>
#include <engine/halfedgeloader.hpp>
#include <engine/material.hpp>
#include <engine/geometry.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/gui.hpp>
#include <engine/scene.hpp>
#include <engine/staticmesh.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/brdf.hpp>
#include <engine/camera.hpp>
#include <engine/skybox.hpp>

#include <iostream>

#define T_CASE(type) case NodeType::type: \
	return #type

using namespace std;

namespace Morpheus {

	Shader* INodeOwner::toShader() 						{ return nullptr; }
	StaticMesh* INodeOwner::toStaticMesh() 				{ return nullptr; }
	Sampler* INodeOwner::toSampler() 					{ return nullptr; }
	Geometry* INodeOwner::toGeometry() 					{ return nullptr; }
	HalfEdgeGeometry* INodeOwner::toHalfEdgeGeometry() 	{ return nullptr; }
	Texture* INodeOwner::toTexture() 					{ return nullptr; }
	Material* INodeOwner::toMaterial() 					{ return nullptr; }
	Scene* INodeOwner::toScene() 						{ return nullptr; }
	Camera* INodeOwner::toCamera() 						{ return nullptr; }
	GuiBase* INodeOwner::toGui() 						{ return nullptr; }
	TransformNode* INodeOwner::toTransform()			{ return nullptr; }
	Skybox* INodeOwner::toSkybox()						{ return nullptr; }
	Framebuffer* INodeOwner::toFramebuffer() 			{ return nullptr; }

	TransformNode* TransformNode::toTransform() 		{ return this; }

	bool NodeMetadata::renderable[(uint32_t)NodeType::END];
	bool NodeMetadata::content[(uint32_t)NodeType::END];

	template <NodeType iType>
	struct InitMetadata {
		static void init(bool* renderable, bool* content) {
			renderable[(uint32_t)iType] = IS_RENDERABLE_<iType>::RESULT;
			content[(uint32_t)iType] = IS_CONTENT_<iType>::RESULT;

			typedef typename OWNER_TYPE_<iType>::RESULT owner_t;

			InitMetadata<(NodeType)((uint32_t)iType + 1)>::init(renderable, content);
		}
	};

	template <>
	struct InitMetadata<NodeType::END> {
		static void init(bool* renderable, bool* content) {
		}
	};

	void NodeMetadata::init() {
		InitMetadata<NodeType::START>::init(renderable, content);
	}

	bool Entity::isUpdatable() const {
		return true;
	}
	bool Entity::isEnabled() const {
		return bEnabled;
	}
	void Entity::setEnabled(const bool value) {
		bEnabled = value;
	}

	std::string nodeTypeString(NodeType t) {
		switch (t) {
			T_CASE(START);
			T_CASE(ENGINE);
			T_CASE(RENDERER);
			T_CASE(UPDATER);
			T_CASE(LAMBERT_SH_COMPUTE_KERNEL);
			T_CASE(GGX_COMPUTE_KERNEL_OLD);
			T_CASE(COOK_TORRANCE_LUT_COMPUTE_KERNEL);
			T_CASE(SCENE_BEGIN);
			T_CASE(CAMERA);
			T_CASE(SCENE_ROOT);
			T_CASE(ENTITY);
			T_CASE(TRANSFORM);
			T_CASE(REGION);
			T_CASE(STATIC_OBJECT_MANAGER);
			T_CASE(DYNAMIC_OBJECT_MANAGER);
			T_CASE(NANOGUI_SCREEN);
			T_CASE(SCENE_END);
			T_CASE(CONTENT_BEGIN);
			T_CASE(CONTENT_MANAGER);
			T_CASE(GEOMETRY);
			T_CASE(MATERIAL);
			T_CASE(SHADER);
			T_CASE(TEXTURE);
			T_CASE(STATIC_MESH);
			T_CASE(CONTENT_END);
			T_CASE(HALF_EDGE_GEOMETRY);
			T_CASE(SAMPLER);
			T_CASE(SKYBOX);
			T_CASE(END);
		default:
			return "UNKNOWN";
		}
	}

	void print(INodeOwner* start)
	{
		size_t depth_mul = 3;

		std::cout << "+ " << nodeTypeString(start->getType()) << std::endl;

		stack<NodeOwnerIteratorF> iters;
		vector<bool> isLast;
		iters.push(start->children());

		if (start->outDegree() > 1)
			isLast.push_back(false);
		else
			isLast.push_back(true);

		while (!iters.empty()) {
			auto& it = iters.top();
			if (it.valid()) {
				INodeOwner* n = it();

				for (int i = 0; i < isLast.size() - 1; ++i) {
					bool b = isLast[i];
					if (b)
						std::cout << " ";
					else
						std::cout << "|";

					for (uint32_t j = 0; j < depth_mul; ++j)
						std::cout << " ";
				}

				std::cout << "o";
				for (uint32_t j = 0; j < depth_mul; ++j)
					cout << "-";
				std::cout << "+ ";
				std::cout << n->node().id() << ": " << nodeTypeString(n->getType());
				
				if (n->isContent())
					std::cout << " \t[" << content()->getSourceString(n) << "]";
				std::cout << std::endl;

				iters.push(n->children());
				it.next();
				if (!it.valid())
					isLast[isLast.size() - 1] = true;
				if (n->outDegree() > 1)
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
			std::cout << "GL_INVALID_ENUM!" << std::endl;
			break;
		case GL_INVALID_VALUE:
			std::cout << "GL_INVALID_VALUE!" << std::endl;
			break;
		case GL_INVALID_OPERATION:
			std::cout << "GL_INVALID_OPERATION!" << std::endl;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "GL_INVALID_FRAMEBUFFER_OPERATOR!" << std::endl;
			break;
		case GL_OUT_OF_MEMORY:
			std::cout << "GL_OUT_OF_MEMORY!" << std::endl;
			break;
		case GL_STACK_UNDERFLOW:
			std::cout << "GL_STACK_UNDERFLOW!" << std::endl;
			break;
		case GL_STACK_OVERFLOW:
			std::cout << "GL_STACK_OVERFLOW!" << std::endl;
			break;
		}
		assert(err == GL_NO_ERROR);
	}

	void checkGLError() {
		printError(glGetError());
	}

	void NodeGraph::applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize) {

		// Update all owners
		for (uint32_t i = 0; i < mapLen; ++i) {
			mOwners[i]->mNodeId = map[mOwners[i]->mNodeId];
		}

		// Call base
		Digraph::applyVertexMap(map, mapLen, newSize);
	}

	void init(INodeOwner* node)
	{
		// Content doesn't belong to a scene and therefore shouldn't be initializable
		if (node->isContent())
			return;

		// Initialize all children
		for (auto it = node->children(); it.valid(); it.next()) {
			init(it());
		}

		// Initialize node after all its children have been initialized
		node->init();

		// If this node is updatable, register it with the updater
		if (node->isUpdatable()) {
			updater()->addChild(node);
		}
	}

	void prune(INodeOwner* start, bool bTopLevel)
	{
		if (start->isContent()) {
			// This is content. It might have multiple users. Unload it later.
			markForUnload(start);
			return;
		} else {
			for (auto it = start->children(); it.valid();) {
				auto node = it();
				it.next();
				prune(node, false);
			}

			if (start->isContent())
				unload(start);
			else {
				Node n = start->node();
				NodeGraph* g = start->graph();
				delete start;
				g->deleteVertex(n);
			}
		}

		if (bTopLevel)
			unloadMarked();
	}

	void prune(INodeOwner* start)
	{
		prune(start, true);
	}

	void prune(const std::string& name) {
		auto node = find(name);
		prune(node);
	}

	Transform Transform::makeIdentity() {
		Transform t;
		t.mRotation = glm::identity<glm::quat>();
		t.mScale = glm::one<glm::vec3>();
		t.mTranslation = glm::zero<glm::vec3>();
		return t;
	}

	Transform Transform::makeTranslation(const glm::vec3& translation) {
		Transform t;
		t.mRotation = glm::identity<glm::quat>();
		t.mScale = glm::one<glm::vec3>();
		t.mTranslation = translation;
		return t;
	}

	Transform Transform::makeRotation(const glm::quat& rotate) {
		Transform t;
		t.mRotation = rotate;
		t.mScale = glm::one<glm::vec3>();
		t.mTranslation = glm::zero<glm::vec3>();
		return t;
	}
}