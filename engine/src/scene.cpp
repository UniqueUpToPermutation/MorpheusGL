#include <engine/scene.hpp>

namespace Morpheus {
	void Scene::dispose() {
		delete this;
	}
}