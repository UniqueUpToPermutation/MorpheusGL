#pragma once

#include "json.hpp"
#include "digraph.hpp"
#include "core.hpp"
#include "shader.hpp"
#include "pool.hpp"
#include "geometry.hpp"
#include "renderqueue.hpp"
#include "engine.hpp"
#include "gui.hpp"

namespace Morpheus {
	
	struct StaticMeshRenderInstance {
		ref<Geometry> mGeometry;
		ref<Transform> mTransform;
	};

	struct ForwardRenderQueue {
		RenderQueue<StaticMeshRenderInstance> mStaticMesh;
		RenderQueue<ref<GuiBase>> mGuis;
	};

	struct ForwardRenderCollectParams {
		ForwardRenderQueue* mQueues;
		std::stack<bool>* mIsStaticStack;
		std::stack<glm::mat4>* mTransformStack;
		StaticTransform* mCurrentStaticTransform;
	};

	struct ForwardRenderDrawParams {

	};

	class ForwardRenderer : public IRenderer {
	private:
		NodeHandle mHandle;
		NodeDataView mNodeDataView;
		ForwardRenderQueue mQueues;
		std::stack<bool> mIsStaticStack;
		std::stack<glm::mat4> mTransformStack;

		void collectRecursive(Node& current, ForwardRenderCollectParams& params);
		void collect(Node& start, ForwardRenderCollectParams& params);
		void draw(const ForwardRenderQueue* queue, const ForwardRenderDrawParams& params);

	public:
		NodeHandle handle() const override;
		RendererType getType() const override;
		void init() override;
		void postGlfwRequests() override;
		void draw(Node& scene) override;
		void dispose() override;

		friend class Engine;
	};
	SET_BASE_TYPE(ForwardRenderer, IRenderer);
}