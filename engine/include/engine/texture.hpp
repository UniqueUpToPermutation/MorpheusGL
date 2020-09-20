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
		uint32_t mLevels;
		GLenum mFormat;
		GLenum mGLTarget;

		void savepngtex2d(const std::string& path) const;
		void savepngcubemap(const std::string& path) const;

	public:
		inline GLuint id() const 			{ return mId; }
		inline TextureType type() const 	{ return mType; }
		inline GLenum target() const 		{ return mGLTarget; }
		inline uint32_t width() const 		{ return mWidth; }
		inline uint32_t height() const 		{ return mHeight; }
		inline uint32_t levels() const 		{ return mLevels; }
		inline uint32_t depth() const 		{ return mDepth; }
		inline GLenum format() const 		{ return mFormat; }

		// Resize the texture. Note that this will destroy everything in the texture.
		void resize(uint32_t width, uint32_t height = 1, uint32_t depth = 1);

		inline void bind(GLenum activeTexture) const {
			glActiveTexture(activeTexture);
			glBindTexture(mGLTarget, mId);
		}

		inline void bindImage(GLuint unit, GLenum access) const {
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(mGLTarget, mId);
			glBindImageTexture(unit, mId, 0, false, 0, access, mFormat);
		}

		void savepng(const std::string& path) const;
		void genMipmaps();

		friend class ContentFactory<Texture>;
	};
	SET_NODE_ENUM(Texture, TEXTURE);

	template <>
	class ContentFactory<Texture> : public IContentFactory {
	private:
		template <bool overrideFormat>
		ref<Texture> loadGliInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		ref<Texture> loadPngInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		ref<Texture> loadInternal(const std::string& source,
			GLenum internalFormat);

	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		ref<Texture> loadGliUnmanaged(const std::string& source);
		ref<Texture> loadPngUnmanaged(const std::string& source);
		ref<Texture> loadGliUnmanaged(const std::string& source,
			GLenum internalFormat);
		ref<Texture> loadPngUnmanaged(const std::string& source,
			GLenum internalFormat);
		void unload(ref<void> ref) override;
		void dispose() override;

		ref<Texture> loadTextureUnmanaged(const std::string& source);
		ref<Texture> loadTextureUnmanaged(const std::string& source, 
			GLenum internalFormat);

		ref<Texture> makeTexture2DUnmanaged(const uint32_t width, const uint32_t height, 
			const GLenum format, const int miplevels = -1);
		Node makeTexture2DUnparented(ref<Texture>* out, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeTexture2D(ref<Texture>* out, Node parent, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeTexture2D(ref<Texture>* out, NodeHandle parent, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		ref<Texture> makeCubemapUnmanaged(const uint32_t width, const uint32_t height, 
			const GLenum format, const int miplevels = -1);
		Node makeCubemapUnparented(ref<Texture>* out, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeCubemap(ref<Texture>* out, Node parent, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Node makeCubemap(ref<Texture>* out, NodeHandle parent, const uint32_t width,
			const uint32_t height, const GLenum format, const int miplevels = -1);
	
		std::string getContentTypeString() const override;
	};
}