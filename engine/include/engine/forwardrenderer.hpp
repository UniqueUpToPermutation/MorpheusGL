#pragma once

#include <engine/json.hpp>
#include <engine/digraph.hpp>
#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/pool.hpp>
#include <engine/geometry.hpp>
#include <engine/renderqueue.hpp>
#include <engine/engine.hpp>
#include <engine/material.hpp>
#include <engine/gui.hpp>
#include <engine/blit.hpp>
namespace Morpheus {
	
	class Camera;

	struct StaticMeshRenderInstance {
		ref<Geometry> mGeometry;
		ref<Transform> mTransform;
		ref<Material> mMaterial;
	};

	struct ForwardRenderQueue {
		RenderQueue<StaticMeshRenderInstance> mStaticMeshes;
		RenderQueue<ref<GuiBase>> mGuis;
	};

	enum class RenderInstanceType {
		STATIC_MESH,
	};

	struct ForwardRenderCollectParams {
		ForwardRenderQueue* mQueues;
		std::stack<bool>* mIsStaticStack;
		std::stack<ref<Transform>>* mTransformStack;
		RenderInstanceType mCurrentRenderType;
		Camera* mRenderCamera;
	};

	struct ForwardRenderDrawParams {
		Camera* mRenderCamera;
	};

	class ForwardRenderer : public IRenderer {
	private:
		NodeHandle mHandle;
		NodeDataView mNodeDataView;
		ForwardRenderQueue mQueues;
		std::stack<bool> mIsStaticStack;
		std::stack<ref<Transform>> mTransformStack;
		std::stack<ref<Material>> mMaterialStack;

		// Debug texture blitting
		ref<Shader> mTextureBlitShader;
		ref<Geometry> mBlitGeometry;
		ref<Sampler> mDebugBlitSampler;
		BlitShaderView mTextureBlitShaderView;

		void collectRecursive(Node& current, ForwardRenderCollectParams& params);
		void collect(Node& start, ForwardRenderCollectParams& params);
		void draw(const ForwardRenderQueue* queue, const ForwardRenderDrawParams& params);
		void makeDebugObjects();

	public:
		NodeHandle handle() const override;
		RendererType getType() const override;
		void init(Node node) override;
		void postGlfwRequests() override;
		void draw(Node scene) override;
		void dispose() override;
		void setClearColor(float r, float g, float b) override;

		void debugBlit(ref<Texture> texture, 
			const glm::vec2& lower,
			const glm::vec2& upper) override;

		friend class Engine;
	};
	SET_BASE_TYPE(ForwardRenderer, IRenderer);
}