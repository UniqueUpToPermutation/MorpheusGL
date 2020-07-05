#include "forwardrenderer.hpp"
#include "engine.hpp"

#include <stack>

#include <nanogui/nanogui.h>

using namespace std;
using namespace glm;

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
		if (NodeMetadata::isSceneChild(desc.type))
			return;

		// Visiting a node on the way down
		switch (desc.type) {
		case NodeType::STATIC_TRANSFORM:
		{
			// For static nodes, transforms have been pre-computed
			params.mCurrentStaticTransform = desc.owner.getAs<StaticTransform>();
			break;
		}
		case NodeType::DYNAMIC_TRANSFORM:
		{
			// Apply the transform on the right
			DynamicTransform* trans = desc.owner.getAs<DynamicTransform>();
			glm::mat4& top = mTransformStack.top();
			params.mTransformStack->push(trans->apply(top));
			break;
		}
		case NodeType::MATERIAL_INSTANCE:
		{
			// Found a material
			break;
		}
		case NodeType::GEOMETRY_INSTANCE:
		{
			// Found some geometry
			break;
		}
		case NodeType::NANOGUI_SCREEN:
		{
			// Found a GUI
			params.mQueues->mGuis.push(desc.owner.getAs<GuiBase>());
			break;
		}
		}

		// If the node is a child of a scene, recursively continue the collection
		for (auto childIt = current.getOutgoingNeighbors(); childIt.valid(); childIt.next()) {
			auto child = childIt();
			collectRecursive(child, params);
		}

		// Visiting a node on the way up
		switch (desc.type) {
		case NodeType::DYNAMIC_TRANSFORM:
			// Pop the transformation from the stack
			params.mTransformStack->pop();
			break;
		}
	}

	void ForwardRenderer::collect(Node& start) {
		mQueues.mGuis.clear();
		mQueues.mStaticMesh.clear();

		mNodeDataView = graph().descs();

		mTransformStack.push(identity<mat4>());
		mIsStaticStack.push(false);
		collectRecursive(start);
		mTransformStack.pop();
		mIsStaticStack.pop();

		assert(mTransformStack.empty());
		assert(mIsStaticStack.empty());
	}

	void ForwardRenderer::draw(const ForwardRenderer::Queues& renderQueues, const ForwardRenderer::DrawParams& params)
	{
		// Just draw GUIs for now
		for (auto guiPtr = renderQueues.mGuis.begin(); guiPtr != renderQueues.mGuis.end(); ++guiPtr) {
			auto screen = (*guiPtr)->screen();
			screen->drawContents();
			screen->drawWidgets();
		}
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
