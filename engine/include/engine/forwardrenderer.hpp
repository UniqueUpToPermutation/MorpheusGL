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

		void collectRecursive(Node& current, ForwardRenderCollectParams& params);
		void collect(Node& start, ForwardRenderCollectParams& params);
		void draw(const ForwardRenderQueue* queue, const ForwardRenderDrawParams& params);

	public:
		NodeHandle handle() const override;
		RendererType getType() const override;
		void init(Node node) override;
		void postGlfwRequests() override;
		void draw(Node scene) override;
		void dispose() override;
		void setClearColor(float r, float g, float b) override;

		friend class Engine;
	};
	SET_BASE_TYPE(ForwardRenderer, IRenderer);
}