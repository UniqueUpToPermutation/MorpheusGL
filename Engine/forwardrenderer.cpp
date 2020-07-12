#include "forwardrenderer.hpp"
#include "engine.hpp"
#include "gui.hpp"
#include "camera.hpp"
#include "scene.hpp"

#include <stack>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

using namespace std;
using namespace glm;

#define DEFAULT_VERSION_MAJOR 3
#define DEFAULT_VERSION_MINOR 3

namespace Morpheus {

	NodeHandle ForwardRenderer::handle() const {
		return mHandle;
	}
	RendererType ForwardRenderer::getType() const {
		return RendererType::FORWARD;
	}

	void ForwardRenderer::collectRecursive(Node& current, ForwardRenderCollectParams& params) {

		auto& desc = mNodeDataView[current];

		// Ignore anything that is not a scene child.
		if (!NodeMetadata::isRenderable(desc.type))
			return;

		// Visiting a node on the way down
		switch (desc.type) {
		case NodeType::SCENE_ROOT:
		{
			auto scene = desc.owner.getAs<Scene>();

			// Set the active camera
			if (!params.mRenderCamera)
				params.mRenderCamera = scene->getActiveCamera();
			break;
		}
		case NodeType::TRANSFORM:
		{
			ref<Transform> newTransform = desc.owner.as<Transform>();
			// Is this transform is static, then it has already been cached
			// Otherwise, cache (evaluate and save) this transform using the
			// last transform on the stack
			if (!params.mIsStaticStack->top())
				if (params.mTransformStack->empty())
					newTransform->cache(identity<mat4>());
				else
					newTransform->cache(params.mTransformStack->top()->mCache);
			// Set the current transform to the one we just found.
			params.mTransformStack->push(newTransform);
			break;
		}
		case NodeType::STATIC_MESH:
		{
			params.mCurrentRenderType = RenderInstanceType::STATIC_MESH;
			break;
		}
		case NodeType::MATERIAL_PROXY:
		{
			// Found a material
			params.mMaterialStack->push(desc.owner.as<Material>());
			break;
		}
		case NodeType::GEOMETRY_PROXY:
		{
			// Found some geometry
			ref<Geometry> geo = desc.owner.as<Geometry>();
			switch (params.mCurrentRenderType) {
			case RenderInstanceType::STATIC_MESH:
				{
					// Push this static mesh onto the queue
					StaticMeshRenderInstance inst;
					inst.mGeometry = geo;
					inst.mMaterial = params.mMaterialStack->top();
					inst.mTransform = params.mTransformStack->top();
					params.mQueues->mStaticMeshes.push(inst);
					break;
				}
			}
			break;
		}
		case NodeType::NANOGUI_SCREEN:
		{
			// Found a GUI
			params.mQueues->mGuis.push(desc.owner.getAs<GuiBase>());
			break;
		}
		case NodeType::CAMERA:
		{
			// Found a camera
			if (!params.mRenderCamera)
				params.mRenderCamera = desc.owner.getAs<ICamera>();
			break;
		}
		}

		// If the node is a child of a scene, recursively continue the collection
		for (auto childIt = current.getChildren(); childIt.valid(); childIt.next()) {
			auto child = childIt();
			collectRecursive(child, params);
		}

		// Visiting a node on the way up
		switch (desc.type) {
		case NodeType::TRANSFORM:
			// Pop the transformation from the stack
			params.mTransformStack->pop();
			break;
		case NodeType::MATERIAL:
			// Pop the material from the stack
			params.mMaterialStack->pop();
			break;
		}
	}

	void ForwardRenderer::collect(Node& start, ForwardRenderCollectParams& params) {
		mQueues.mGuis.clear();
		mQueues.mStaticMeshes.clear();
		mNodeDataView = graph()->descs();

		params.mQueues = &mQueues;
		params.mTransformStack = &mTransformStack;
		params.mMaterialStack = &mMaterialStack;
		params.mIsStaticStack = &mIsStaticStack;
		params.mRenderCamera = nullptr;

		params.mIsStaticStack->push(false);
		collectRecursive(start, params);
		params.mIsStaticStack->pop();

		assert(mTransformStack.empty());
		assert(mIsStaticStack.empty());
	}

	void ForwardRenderer::draw(const ForwardRenderQueue* queue, const ForwardRenderDrawParams& params)
	{
		int width;
		int height;
		glfwGetFramebufferSize(window(), &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		mat4 view = identity<mat4>();
		mat4 projection = identity<mat4>();
		vec3 eye = zero<vec3>();

		if (params.mRenderCamera) {
			view = params.mRenderCamera->view();
			projection = params.mRenderCamera->projection();
			eye = params.mRenderCamera->eye();
		}

		// Draw static meshes
		for (auto meshPtr = queue->mStaticMeshes.begin(); meshPtr != queue->mStaticMeshes.end(); ++meshPtr) {
			auto& material = meshPtr->mMaterial;
			auto& transform = meshPtr->mTransform;
			auto& geo = meshPtr->mGeometry;

			GL_ASSERT;

			auto shader = material->shader();
			auto& shaderRenderView = shader->renderView();

			mat4 world = transform->mCache;
			mat4 worldInvTranspose = glm::transpose(glm::inverse(world));

			// Set renderer related things
			glUseProgram(shader->id());
			shaderRenderView.mWorld.set(world);
			shaderRenderView.mView.set(view);
			shaderRenderView.mProjection.set(projection);
			shaderRenderView.mWorldInverseTranspose.set(worldInvTranspose);
			shaderRenderView.mEyePosition.set(eye);
			GL_ASSERT;
			// Set individual material parameters
			material->uniformAssignments().assign();
			GL_ASSERT;
			// Bind the geometry's vertex arary and draw the geometry
			glBindVertexArray(geo->vertexArray());
			GL_ASSERT;
			glDrawElements(geo->elementType(), geo->elementCount(),
				geo->indexType(), nullptr);
			GL_ASSERT;
		}

		// Just draw GUIs last for now
		for (auto guiPtr = queue->mGuis.begin(); guiPtr != queue->mGuis.end(); ++guiPtr) {
			auto screen = (*guiPtr)->screen();
			screen->drawContents();
			screen->drawWidgets();
		}
	}

	void ForwardRenderer::draw(Node& scene) {
		ForwardRenderCollectParams collectParams;
		collectParams.mQueues = &mQueues;
		collectParams.mIsStaticStack = &mIsStaticStack;
		collectParams.mTransformStack = &mTransformStack;

		ForwardRenderDrawParams drawParams;

		collect(scene, collectParams);

		drawParams.mRenderCamera = collectParams.mRenderCamera;
		draw(&mQueues, drawParams);
	}
	void ForwardRenderer::postGlfwRequests() {
		auto& glConfig = (*config())["opengl"];

		uint32_t majorVersion = DEFAULT_VERSION_MAJOR;
		uint32_t minorVersion = DEFAULT_VERSION_MINOR;
		glConfig["v_major"].get_to(majorVersion);
		glConfig["v_minor"].get_to(minorVersion);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_STENCIL_BITS, 8);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	}
	void ForwardRenderer::init()
	{
		// Set VSync on
		glfwSwapInterval(1); 

		GL_ASSERT;
		glEnable(GL_DEPTH_TEST);
		GL_ASSERT;
		glEnable(GL_STENCIL_TEST);
		GL_ASSERT;
		glEnable(GL_CULL_FACE);
		GL_ASSERT;
	}
	void ForwardRenderer::dispose() {

	}

}
