#pragma once

#include "json.hpp"
#include "digraph.hpp"
#include "core.hpp"
#include "shader.hpp"
#include "pool.hpp"
#include "geometry.hpp"
#include "renderqueue.hpp"

namespace Morpheus {
	
	class IRenderer : public IDisposable {
	public:
		virtual void init() = 0;
		virtual void draw(Node& scene) = 0;
		virtual NodeHandle handle() const = 0;
		virtual RendererType getType() const = 0;
		inline Node node() const { return graph()[handle()]; }
	};

	struct StaticMeshRenderInstance {
		ref<Geometry> mGeometry;
		ref<Transform> mTransform;
	};

	class ForwardRenderer : public IRenderer {
	private:
		NodeHandle mHandle;
		NodeDataView mNodeDataView;
		std::stack<bool> mIsStaticStack;
		std::stack<glm::mat4> mTransformStack;
		StaticTransform* mCurrentStaticTransform;

		struct Queues {
			RenderQueue<StaticMeshRenderInstance> mStaticMesh;
		} mQueues;

		struct DrawParams {

		} mDrawParams;

		void collectRecursive(Node& current);
		void collect(Node& start);
		void draw(const Queues& renderQueues, const DrawParams& params);

	public:
		NodeHandle handle() const override;
		RendererType getType() const override;
		void init() override;
		void draw(Node& scene) override;
		void dispose() override;
	};
}