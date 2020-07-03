#pragma once

#include "json.hpp"
#include "digraph.hpp"
#include "core.hpp"
#include "shader.hpp"
#include "pool.hpp"
#include "geometry.hpp"
#include "renderqueue.hpp"

namespace Morpheus {

	struct StaticMeshRenderInstance {
		ref<Geometry> mGeometry;
		ref<Transform> mTransform;
	};

	class ForwardRenderer : public IRenderer {
	private:
		NodeHandle mHandle;

		struct Queues {
			RenderQueue<StaticMeshRenderInstance> mStaticMesh;
		} mQueues;

		struct DrawParams {

		} mDrawParams;

		void collect(DigraphVertex& scene, Queues* renderQueues);
		void draw(const Queues& renderQueues, const DrawParams& params);

	public:
		NodeHandle handle() const override;
		RendererType getType() const override;
		void init() override;
		void draw(DigraphVertex& scene) override;
		void dispose() override;
	};
}