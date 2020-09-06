#pragma once

#include "texture.hpp"

namespace Morpheus {
	class Skybox {
	public:
		static ref<Texture> getCubemap(Node skybox);
		static void setCubemap(Node skybox, ref<Texture> cubemap);
		static Node make(ref<Texture> cubemap, Node parent);
	};
}