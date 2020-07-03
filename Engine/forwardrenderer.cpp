#include "forwardrenderer.hpp"
#include "engine.hpp"

#include <stack>

using namespace std;
using namespace glm;

namespace Morpheus {

	NodeHandle ForwardRenderer::handle() const {
		return mHandle;
	}
	RendererType ForwardRenderer::getType() const {
		return RendererType::FORWARD;
	}

	void ForwardRenderer::collectRecursive(Node& current) {

		auto& desc = mNodeDataView[current];

		// Ignore anything that is not a scene child.
		if (NodeMetadata::isSceneChild(desc.type))
			return;

		// Visiting a node on the way down
		switch (desc.type) {
		case NodeType::STATIC_TRANSFORM:
		{
			// For static nodes, transforms have been pre-computed
			mCurrentStaticTransform = desc.owner.getAs<StaticTransform>();
			break;
		}
		case NodeType::DYNAMIC_TRANSFORM:
		{
			// Apply the transform on the right
			DynamicTransform* trans = desc.owner.getAs<DynamicTransform>();
			glm::mat4& top = mTransformStack.top();
			mTransformStack.push(trans->apply(top));
			break;
		}
		case NodeType::MATERIAL_INSTANCE:
		{
			break;
		}
		}

		// If the node is a child of a scene, recursively continue the collection
		for (auto childIt = current.getOutgoingNeighbors(); childIt.valid(); childIt.next()) {
			auto child = childIt();
			collectRecursive(child);
		}

		// Visiting a node on the way up
		switch (desc.type) {
		case NodeType::DYNAMIC_TRANSFORM:
			// Pop the transformation from the stack
			mTransformStack.pop();
			break;
		}
	}

	void ForwardRenderer::collect(Node& start) {
		mNodeDataView = graph().descs();

		mTransformStack.push(identity<mat4>());
		mIsStaticStack.push(false);
		collectRecursive(start);
		mTransformStack.pop();
		mIsStaticStack.pop();

		assert(mTransformStack.empty());
		assert(mIsStaticStack.empty());
	}

	void ForwardRenderer::draw(Node& scene) {
		collect(scene);
		draw(mQueues, mDrawParams);
	}
	void ForwardRenderer::init()
	{
		auto v = graph().addNode(this, engine().handle());
		mHandle = graph().issueHandle(v);
	}
	void ForwardRenderer::dispose() {

	}

}
