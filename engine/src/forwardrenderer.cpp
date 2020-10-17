#include <engine/forwardrenderer.hpp>
#include <engine/engine.hpp>
#include <engine/gui.hpp>
#include <engine/camera.hpp>
#include <engine/scene.hpp>
#include <engine/staticmesh.hpp>
#include <engine/blit.hpp>
#include <engine/sampler.hpp>
#include <engine/framebuffer.hpp>

#include <stack>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

using namespace std;
using namespace glm;

#define DEFAULT_VERSION_MAJOR 4
#define DEFAULT_VERSION_MINOR 5

namespace Morpheus {

	RendererType ForwardRenderer::getRendererType() const {
		return RendererType::FORWARD;
	}

	void ForwardRenderer::collectRecursive(INodeOwner* current, ForwardRenderCollectParams& params) {

		// Ignore anything that is not a scene child.
		if (!current->isRenderable())
			return;

		// Visiting a node on the way down
		switch (current->getType()) {
		case NodeType::SCENE_ROOT:
		{
			auto scene = current->toScene();

			// Set the active camera
			if (params.mRenderCamera)
				params.mRenderCamera = scene->getActiveCamera();
			break;
		}
		case NodeType::TRANSFORM:
		{
			auto newTransform = current->toTransform();
			// Is this transform is static, then it has already been cached
			// Otherwise, cache (evaluate and save) this transform using the
			// last transform on the stack
			if (!params.mIsStaticStack->top()) {
				if (params.mTransformStack->empty()) {
					newTransform->mTransform.cache(identity<mat4>());
				} else {
					newTransform->mTransform.cache(params.mTransformStack->top()->mCache);
				}
			} 
			// Set the current transform to the one we just found.
			params.mTransformStack->push(&newTransform->mTransform);
			break;
		}
		case NodeType::STATIC_MESH:
		{
			StaticMeshRenderInstance inst;
			inst.mTransform = params.mTransformStack->top();
			inst.mStaticMesh = current->toStaticMesh();
			params.mQueues->mStaticMeshes.push(inst);
			break;
		}
		case NodeType::NANOGUI_SCREEN:
		{
			// Found a GUI
			params.mQueues->mGuis.push(current->toGui());
			break;
		}
		case NodeType::CAMERA:
		{
			// Found a camera
			if (!params.mRenderCamera)
				params.mRenderCamera = current->toCamera();
			break;
		}
		case NodeType::SKYBOX:
		{
			if (!params.mSkybox)
				params.mSkybox = current->toSkybox();
			break;
		}
		default:
			break;
		}

		// If the node is a child of a scene, recursively continue the collection
		for (auto childIt = current->children(); childIt.valid(); childIt.next()) {
			collectRecursive(childIt(), params);
		}

		// Visiting a node on the way up
		switch (current->getType()) {
		case NodeType::TRANSFORM:
			// Pop the transformation from the stack
			params.mTransformStack->pop();
			break;
		default:
			break;
		}
	}

	void ForwardRenderer::collect(INodeOwner* start, ForwardRenderCollectParams& params) {
		mQueues.mGuis.clear();
		mQueues.mStaticMeshes.clear();

		params.mQueues = &mQueues;
		params.mTransformStack = &mTransformStack;
		params.mIsStaticStack = &mIsStaticStack;
		params.mRenderCamera = nullptr;
		params.mSkybox = nullptr;

		params.mIsStaticStack->push(false);
		collectRecursive(start, params);
		params.mIsStaticStack->pop();

		assert(mTransformStack.empty());
		assert(mIsStaticStack.empty());
	}

	void ForwardRenderer::makeDebugObjects() {
		// Create the texture blit shader (managed)
		mDebugBlitSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, this);

		// Create blit geometry and shader (
		mBlitGeometry = makeBlitGeometry(this);
		mTextureBlitShader = makeBlitShader(this, &mTextureBlitShaderView);
	}

	void ForwardRenderer::resetFramebuffer() {
		int width, height;
		getFramebufferSize(&width, &height);

		if (mTargetBuffer) {
			mTargetBuffer->resize(width, height);
		} else {
			mTargetBuffer = getFactory<Framebuffer>()->makeFramebuffer(this, width, height, 
				GL_RGBA8, GL_DEPTH24_STENCIL8, 1);
		}

		if (mCurrentSettings.mMSAASamples > 1) {
			if (!mMultisampleTargetBuffer) {
				mMultisampleTargetBuffer = getFactory<Framebuffer>()->makeFramebuffer(this, width, height, 
					GL_RGBA8, GL_DEPTH24_STENCIL8, mCurrentSettings.mMSAASamples);
			}
			else {
				mMultisampleTargetBuffer->resize(width, height, mCurrentSettings.mMSAASamples);
			}
		} else {
			if (mMultisampleTargetBuffer) {
				unload(mMultisampleTargetBuffer);
				mMultisampleTargetBuffer = nullptr;
			}
		}

		GL_ASSERT;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GL_ASSERT;
	}

	RenderSettings ForwardRenderer::readSetingsFromConfig(const nlohmann::json& config) {
		RenderSettings result;
		result.mAnisotropySamples = config.value("anisotropy_samples", 1);
		result.mMSAASamples = config.value("msaa_samples", 1);
		return result;
	}

	ForwardRenderer::ForwardRenderer() : 
		mCubemapSampler(nullptr), 
		mTextureSampler(nullptr),
		mDebugBlitSampler(nullptr),
		mBlitGeometry(nullptr),
		mTextureBlitShader(nullptr),
		mMultisampleTargetBuffer(nullptr),
		mTargetBuffer(nullptr) {

		mOnFramebufferResize = [this](GLFWwindow* window, int width, int height) {
			this->resetFramebuffer();
			
			return false;
		};

		input()->registerTarget(this, InputPriority::ELEVATED);
		input()->bindFramebufferSizeEvent(this, &mOnFramebufferResize);
	}

	ForwardRenderer::~ForwardRenderer() {
		input()->unbindFramebufferSizeEvent(this);
		input()->unregisterTarget(this);
	}

	void ForwardRenderer::setRenderSettings(const RenderSettings& settings) {
		// Create or update cubemap sampler with render settings
		if (!mCubemapSampler) {
			ContentExtParams<Sampler> samplerParams = makeSamplerParams(SamplerPrototype::TRILINEAR_CLAMP);
			samplerParams.mAnisotropy = settings.mAnisotropySamples;
			mCubemapSampler = loadEx<Sampler>(RENDERER_CUBEMAP_SAMPLER_SRC, samplerParams);
		} else {
			mCubemapSampler->setAnisotropy(settings.mAnisotropySamples);
		}

		// Create or update texture sampler with render settings
		if (!mTextureSampler) {
			ContentExtParams<Sampler> samplerParams = makeSamplerParams(SamplerPrototype::TRILINEAR_TILE);
			samplerParams.mAnisotropy = settings.mAnisotropySamples;
			mTextureSampler = loadEx<Sampler>(RENDERER_TEXTURE_SAMPLER_SRC, samplerParams);
		} else {
			mTextureSampler->setAnisotropy(settings.mAnisotropySamples);
		}

		mCurrentSettings = settings;
	}

	RenderSettings ForwardRenderer::getRenderSettings() const {
		return mCurrentSettings;
	}

	void ForwardRenderer::draw(ForwardRenderQueue* queue, const ForwardRenderDrawParams& params)
	{
		int width;
		int height;
		glfwGetFramebufferSize(window(), &width, &height);

		GL_ASSERT;

		Framebuffer* renderTarget = mTargetBuffer;
		if (mMultisampleTargetBuffer)
			renderTarget = mMultisampleTargetBuffer;
		renderTarget->bind();
		GL_ASSERT;

		glViewport(0, 0, width, height);
		if (params.mSkybox)
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		else
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		mat4 view = identity<mat4>();
		mat4 projection = identity<mat4>();
		vec3 eye = zero<vec3>();

		if (params.mRenderCamera) {
			view = params.mRenderCamera->view();
			projection = params.mRenderCamera->projection();
			eye = params.mRenderCamera->eye();
		}

		// Setup GL parameters
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDisable(GL_BLEND);

		// Draw static meshes
		for (auto meshPtr = queue->mStaticMeshes.begin(); meshPtr != queue->mStaticMeshes.end(); ++meshPtr) {
			auto material = meshPtr->mStaticMesh->getMaterial();
			auto transform = meshPtr->mTransform;
			auto geo = meshPtr->mStaticMesh->getGeometry();

			GL_ASSERT;

			auto shader = material->shader();
			auto& shaderRenderView = shader->renderView();

			mat4 world = transform->mCache;
			mat4 worldInvTranspose = glm::transpose(glm::inverse(world));

			// Set renderer related things
			shader->bind();
			shaderRenderView.mWorld.set(world);
			shaderRenderView.mView.set(view);
			shaderRenderView.mProjection.set(projection);
			shaderRenderView.mWorldInverseTranspose.set(worldInvTranspose);
			shaderRenderView.mEyePosition.set(eye);
			GL_ASSERT;
			// Set individual material parameters
			material->uniformAssignments().assign();
			GL_ASSERT;
			// Assign textures
			material->samplerAssignments().assign();
			GL_ASSERT;
			// Bind the geometry's vertex arary and draw the geometry
			glBindVertexArray(geo->vertexArray());
			glBindBuffer(GL_ARRAY_BUFFER, geo->vertexBuffer());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->indexBuffer());
			GL_ASSERT;
			glDrawElements(geo->elementType(), geo->elementCount(),
				geo->indexType(), nullptr);
			GL_ASSERT;
		}

		// Draw skybox
		if (params.mSkybox) {
			params.mSkybox->prepare(view, projection, eye);
			GL_ASSERT;
			glBindVertexArray(mBlitGeometry->vertexArray());
			glBindBuffer(GL_ARRAY_BUFFER, mBlitGeometry->vertexBuffer());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBlitGeometry->indexBuffer());
			glDrawElements(mBlitGeometry->elementType(), mBlitGeometry->elementCount(),
					mBlitGeometry->indexType(), nullptr);
			GL_ASSERT;
		}

		// Copy render target to back buffer
		renderTarget->blitToBackBuffer(GL_COLOR_BUFFER_BIT | 
			GL_DEPTH_BUFFER_BIT | 
			GL_STENCIL_BUFFER_BIT);

		// Just draw GUIs last for now
		glBindVertexArray(0);
		glUseProgram(0);
		for (auto guiPtr = queue->mGuis.begin(); guiPtr != queue->mGuis.end(); ++guiPtr) {
			auto screen = (*guiPtr)->screen();
			screen->drawContents();
			screen->drawWidgets();
		}
	}

	void ForwardRenderer::draw(INodeOwner* scene) {
		ForwardRenderCollectParams collectParams;
		collectParams.mQueues = &mQueues;
		collectParams.mIsStaticStack = &mIsStaticStack;
		collectParams.mTransformStack = &mTransformStack;

		ForwardRenderDrawParams drawParams;

		collect(scene, collectParams);

		drawParams.mRenderCamera = collectParams.mRenderCamera;
		drawParams.mSkybox = collectParams.mSkybox;

		draw(&mQueues, drawParams);
	}

	void ForwardRenderer::postGlfwRequests() {
		auto& config_ = *config();
		nlohmann::json glConfig;
		if (config_.contains("opengl"))
			glConfig = config_["opengl"];
		else
			glConfig = nlohmann::json::value_t::object;

		uint32_t majorVersion = glConfig.value("v_major", DEFAULT_VERSION_MAJOR);
		uint32_t minorVersion = glConfig.value("v_minor", DEFAULT_VERSION_MINOR);

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
		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);

		makeDebugObjects();

		// Read render settings from config and apply
		auto& config_ = *config();
		RenderSettings settings;
		if (config_.contains("render_settings")) {
			auto& render_settings_config = config_["render_settings"];
			settings = readSetingsFromConfig(render_settings_config);
		}
		else {
			nlohmann::json j_object_empty(nlohmann::json::value_t::object);
			settings = readSetingsFromConfig(j_object_empty);
		}

		setRenderSettings(settings);
		resetFramebuffer();
	}

	void ForwardRenderer::setClearColor(float r, float g, float b) {
		glClearColor(r, g, b, 1.0f);
	}

	void ForwardRenderer::blit(Texture* texture, 
			const glm::vec2& lower,
			const glm::vec2& upper,
			Shader* shader,
			BlitShaderView* shaderView) {
		glDisable(GL_DEPTH_TEST);

		int width;
		int height;
		glfwGetFramebufferSize(window(), &width, &height);

		glm::vec2 lower_normalized = lower;
		glm::vec2 upper_normalized = upper;
		glm::vec2 origin(-1.0f, -1.0f);
		glm::vec2 scale(2.0f / (float)width, 2.0f / (float)height);
		lower_normalized *= scale;
		upper_normalized *= scale;
		lower_normalized += origin;
		upper_normalized += origin;

		lower_normalized.y *= -1.0;
		upper_normalized.y *= -1.0;

		if (!shader) {
			shader = mTextureBlitShader;
			shaderView = &mTextureBlitShaderView;
		}

		shader->bind();
		shaderView->mLower.set(lower_normalized);
		shaderView->mUpper.set(upper_normalized);
		shaderView->mBlitTexture.set(texture, mDebugBlitSampler);

		glBindVertexArray(mBlitGeometry->vertexArray());
		glBindBuffer(GL_ARRAY_BUFFER, mBlitGeometry->vertexBuffer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBlitGeometry->indexBuffer());
		glDrawElements(mBlitGeometry->elementType(), mBlitGeometry->elementCount(),
				mBlitGeometry->indexType(), nullptr);
		GL_ASSERT;
	}
}
