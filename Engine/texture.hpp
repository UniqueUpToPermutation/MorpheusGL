#pragma once

#include <glad/glad.h>

#include "content.hpp"

namespace Morpheus {
	enum class TextureType {
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		CUBE_MAP,
		TEXTURE_3D,
		CUBE_MAP_ARRAY
	};

	class Texture {
	private:
		GLuint mId;
		TextureType mType;

	public:
		inline GLuint id() const { return mId; }
		inline TextureType type() const { return mType; }

		friend class ContentFactory<Texture>;
	};
	SET_NODE_ENUM(Texture, TEXTURE);

	template <>
	class ContentFactory<Texture> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		void makeTexture2DUnmanaged(const uint32_t width, const uint32_t height, const GLuint format, const int miplevels = -1);
		void makeTexture2D(const uint32_t width, const uint32_t height, const GLuint format, const int miplevels = -1);
		void makeCubemapUnmanaged(const uint32_t width, const uint32_t height, const GLuint format, const int miplevels = -1);
		void makeCubemap(const uint32_t width, const uint32_t height, const GLuint format, const int miplevels = -1);
	};
}