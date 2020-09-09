#pragma once

#include <glad/glad.h>

#include <engine/content.hpp>

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
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDepth;
		GLenum mFormat;

		void savepngtex2d(const std::string& path);
		void savepngcubemap(const std::string& path);

	public:
		inline GLuint id() const { return mId; }
		inline TextureType type() const { return mType; }
		inline uint32_t width() const { return mWidth; }
		inline uint32_t height() const { return mHeight; }
		inline uint32_t depth() const { return mDepth; }
		inline GLenum format() const { return mFormat; }

		void savepng(const std::string& path);
		void genMipmaps();

		friend class ContentFactory<Texture>;
	};
	SET_NODE_ENUM(Texture, TEXTURE);

	template <>
	class ContentFactory<Texture> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		ref<Texture> loadgli(const std::string& source);
		ref<Texture> loadpng(const std::string& source);
		void unload(ref<void>& ref) override;
		void dispose() override;

		ref<Texture> makeTexture2DUnmanaged(const uint32_t width, const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeTexture2D(ref<Texture>* out, const uint32_t width, const uint32_t height, const GLenum format, const int miplevels = -1);
		ref<Texture> makeCubemapUnmanaged(const uint32_t width, const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeCubemap(ref<Texture>* out, const uint32_t width, const uint32_t height, const GLenum format, const int miplevels = -1);
	};
}