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
	};

	struct ForwardRenderDrawParams {
		Camera* mRenderCamera;
	};

	class ForwardRenderer : public IRenderer {
	private:
		ForwardRenderQueue mQueues;
		std::stack<bool> mIsStaticStack;
		std::stack<Transform*> mTransformStack;
		std::stack<Material*> mMaterialStack;

		// Debug texture blitting
		Shader* mTextureBlitShader;
		Geometry* mBlitGeometry;
		Sampler* mDebugBlitSampler;
		BlitShaderView mTextureBlitShaderView;

		void collectRecursive(INodeOwner* current, ForwardRenderCollectParams& params);
		void collect(INodeOwner* start, ForwardRenderCollectParams& params);
		void draw(ForwardRenderQueue* queue, const ForwardRenderDrawParams& params);
		void makeDebugObjects();

	public:
		~ForwardRenderer() override;

		RendererType getRendererType() const override;
		void init() override;
		void postGlfwRequests() override;
		void draw(INodeOwner* scene) override;
		void setClearColor(float r, float g, float b) override;

		void debugBlit(Texture* texture, 
			const glm::vec2& lower,
			const glm::vec2& upper) override;

		friend class Engine;
	};
	SET_BASE_TYPE(ForwardRenderer, IRenderer);
}