#include "forwardrenderer.hpp"
#include "engine.hpp"

namespace Morpheus {

	NodeHandle ForwardRenderer::handle() const {
		return mHandle;
	}
	RendererType ForwardRenderer::getType() const {
		return RendererType::FORWARD;
	}
	void ForwardRenderer::draw(DigraphVertex& scene) {

	}
	void ForwardRenderer::init()
	{
		auto v = graph().addNode(this, engine().handle());
		mHandle = graph().issueHandle(v);
	}
	void ForwardRenderer::dispose() {

	}

}
