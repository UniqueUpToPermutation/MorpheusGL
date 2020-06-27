#include "content.hpp"
#include "shader.hpp"

namespace Morpheus {
	ContentManager::ContentManager() {
		auto v = graph().addNode(this, engine().handle());
		mHandle = graph().issueHandle(v);
		mSources = graph().createVertexLookup<std::string>("content_src");

		// Make shader factory
		addFactory<IShader>();
	}
}