#include "content.hpp"
#include "shader.hpp"

namespace Morpheus {
	ContentManager::ContentManager() {
		auto v = graph().addNode(this, engine().handle());
		handle_ = graph().issueHandle(v);
		sources_ = graph().createVertexLookup<std::string>("content_src");

		// Make shader factory
		addFactory<IShader>();
	}
}