#pragma once

#include <engine/json.hpp>
#include <engine/digraph.hpp>
#include <engine/core.hpp>
#include <engine/camera.hpp>
#include <engine/shader.hpp>
#include <engine/pool.hpp>
#include <engine/geometry.hpp>
#include <engine/renderqueue.hpp>
#include <engine/engine.hpp>
#include <engine/material.hpp>
#include <engine/gui.hpp>
#include <engine/blit.hpp>
#include <engine/skybox.hpp>
namespace Morpheus {
	struct StaticMeshRenderInstance {
		StaticMesh* mStaticMesh;
		Transform* mTransform;
	};

	struct ForwardRenderQueue {
		RenderQueue<StaticMeshRenderInstance> mStaticMeshes;
		RenderQueue<GuiBase*> mGuis;
	};

	enum class RenderInstanceType {
		STATIC_MESH,
	};

	struct ForwardRenderCollectParams {
		ForwardRenderQueue* mQueues;
		std::stack<bool>* mIsStaticStack;
		std::stack<Transform*>* mTransformStack;
		RenderInstanceType mCurrentRenderType;
		Camera* mRenderCamera;
		Skybox* mSkybox;
	};

	struct ForwardRenderDrawParams {
		Camera* mRenderCamera;
		Skybox* mSkybox;
	};

	class ForwardRenderer : public IRenderer {
	private:
		ForwardRenderQueue mQueues;
		std::stack<bool> mIsStaticStack;
		std::stack<Transform*> mTransformStack;
		std::stack<Material*> mMaterialStack;
		RenderSettings mCurrentSettings;

		f_framebuffer_size_capture_t mOnFramebufferResize;

		// Debug texture blitting
		Shader* mTextureBlitShader;
		Geometry* mBlitGeometry;
		Sampler* mDebugBlitSampler;
		BlitShaderView mTextureBlitShaderView;

		Sampler* mTextureSampler;
		Sampler* mCubemapSampler;

		// Framebuffer
		Framebuffer* mMultisampleTargetBuffer;
		Framebuffer* mTargetBuffer;

		void collectRecursive(INodeOwner* current, ForwardRenderCollectParams& params);
		void collect(INodeOwner* start, ForwardRenderCollectParams& params);
		void draw(ForwardRenderQueue* queue, const ForwardRenderDrawParams& params);
		void makeDebugObjects();
		void resetFramebuffer();
		RenderSettings readSetingsFromConfig(const nlohmann::json& config);

	public:
		ForwardRenderer();
		~ForwardRenderer();

		void setRenderSettings(const RenderSettings& settings) override;
		RenderSettings getRenderSettings() const override;

		RendererType getRendererType() const override;
		void init() override;
		void postGlfwRequests() override;
		void draw(INodeOwner* scene) override;
		void setClearColor(float r, float g, float b) override;

		void blit(Texture* texture,
			const glm::vec2& lower,
			const glm::vec2& upper,
			Shader* shader,
			BlitShaderView* shaderView) override;

		friend class Engine;
	};
	SET_BASE_TYPE(ForwardRenderer, IRenderer);
}