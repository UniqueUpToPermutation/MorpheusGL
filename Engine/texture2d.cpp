#include "texture2d.hpp"

#include <SOIL/SOIL.h>

namespace Morpheus {
	ref<void> ContentFactory<Texture2D>::load(const std::string& source, Node& loadInto) {
		Texture2D* tex = new Texture2D();
		tex->mId = SOIL_load_OGL_texture(source.c_str(), 0, 0, 0);
		return ref<void>(tex);
	}
	void ContentFactory<Texture2D>::unload(ref<void>& ref) {
		auto tex = ref.reinterpretGet<Texture2D>();
		glDeleteTextures(1, &tex->mId);
		delete tex;
	}
	void ContentFactory<Texture2D>::dispose() {
		delete this;
	}
}