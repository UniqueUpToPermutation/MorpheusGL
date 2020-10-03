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

	class Texture : public INodeOwner {
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
		inline Texture() : INodeOwner(NodeType::TEXTURE), 
			mId(0), mWidth(0), mHeight(0), mDepth(0), mLevels(0) {
		}

		Texture* toTexture() override;

		inline GLuint id() const 				{ return mId; }
		inline TextureType textureType() const 	{ return mType; }
		inline GLenum target() const 			{ return mGLTarget; }
		inline uint32_t width() const 			{ return mWidth; }
		inline uint32_t height() const 			{ return mHeight; }
		inline uint32_t levels() const 			{ return mLevels; }
		inline uint32_t depth() const 			{ return mDepth; }
		inline GLenum format() const 			{ return mFormat; }

		// Resize the texture. Note that this will destroy everything in the texture.
		void resize(uint32_t width, uint32_t height = 1, uint32_t depth = 1);

		inline void bind(GLuint unit) const {
			glActiveTexture(GL_TEXTURE0 + unit);
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
		Texture* loadGliInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		Texture* loadPngInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		Texture* loadInternal(const std::string& source,
			GLenum internalFormat);

	public:
		INodeOwner* load(const std::string& source, Node loadInto) override;
		Texture* loadGliUnmanaged(const std::string& source);
		Texture* loadPngUnmanaged(const std::string& source);
		Texture* loadGliUnmanaged(const std::string& source,
			GLenum internalFormat);
		Texture* loadPngUnmanaged(const std::string& source,
			GLenum internalFormat);
		void unload(INodeOwner* ref) override;
		void dispose() override;

		Texture* loadTextureUnmanaged(const std::string& source);
		Texture* loadTextureUnmanaged(const std::string& source, 
			GLenum internalFormat);

		Texture* makeTexture2DUnmanaged(const uint32_t width, const uint32_t height, 
			const GLenum format, const int miplevels = -1);
		Texture* makeTexture2DUnparented(const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Texture* makeTexture2D(INodeOwner* parent, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Texture* makeCubemapUnmanaged(const uint32_t width, const uint32_t height, 
			const GLenum format, const int miplevels = -1);
		Texture* makeCubemapUnparented(const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
		Texture* makeCubemap(INodeOwner* parent, const uint32_t width, 
			const uint32_t height, const GLenum format, const int miplevels = -1);
	
		std::string getContentTypeString() const override;
	};
}