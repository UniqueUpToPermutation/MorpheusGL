#pragma once

#include <glad/glad.h>

#include <engine/content.hpp>

namespace Morpheus {

	inline uint mipCount(const uint width, const uint height) {
		return 1 + std::floor(std::log2(std::max(width, height)));
	}

	inline uint mipCount(const uint width, const uint height, const uint depth) {
		return 1 + std::floor(std::log2(std::max(width, std::max(height, depth))));
	}

	enum class TextureType {
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_MULTISAMPLE,
		TEXTURE_2D_ARRAY,
		TEXTURE_2D_ARRAY_MULTISAMPLE,
		CUBE_MAP,
		TEXTURE_3D,
		CUBE_MAP_ARRAY
	};

	class Texture : public INodeOwner {
	private:
		GLuint mId;
		TextureType mType;
		uint mWidth;
		uint mHeight;
		uint mDepth;
		uint mLevels;
		uint mSamples;
		GLenum mFormat;
		GLenum mGLTarget;

		void savepngtex2d(const std::string& path) const;
		void savepngcubemap(const std::string& path) const;

	public:
		inline Texture() : INodeOwner(NodeType::TEXTURE), 
			mId(0), mWidth(0), mHeight(0), mDepth(0), mLevels(0), mSamples(1) {
		}

		Texture* toTexture() override;

		inline GLuint id() const 				{ return mId; }
		inline TextureType textureType() const 	{ return mType; }
		inline GLenum target() const 			{ return mGLTarget; }
		inline uint width() const 				{ return mWidth; }
		inline uint height() const 				{ return mHeight; }
		inline uint levels() const 				{ return mLevels; }
		inline uint depth() const 				{ return mDepth; }
		inline uint samples() const 			{ return mSamples; }
		inline GLenum format() const 			{ return mFormat; }
		inline glm::ivec2 size2d() const 		{ return glm::ivec2(mWidth, mHeight); }
		inline glm::ivec3 size3d() const 		{ return glm::ivec3(mWidth, mHeight, mDepth); }

		// Resize the texture. Note that this will destroy everything in the texture.
		void resize(uint width, uint height = 1, uint depth = 1, 
			int miplevels = -1, int samples = -1);

		inline void bind(GLuint unit) const {
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(mGLTarget, mId);
		}

		inline void bindImage(GLuint unit, GLenum access, GLint level = 0) const {
			/*glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(mGLTarget, mId);*/
			glBindImageTexture(unit, mId, level, false, 0, access, mFormat);
		}

		void savepng(const std::string& path) const;
		void genMipmaps();

		friend class ContentFactory<Texture>;
	};
	SET_NODE_ENUM(Texture, TEXTURE);

	template <>
	struct ContentExtParams<Texture> {
		GLenum mInternalFormat;
		bool bGenerateMips;

		ContentExtParams(GLenum internalFormat = 0, bool generateMips = true) : 
			mInternalFormat(internalFormat), 
			bGenerateMips(generateMips) {
		}
	};

	enum class TextureLoader {
		GLI,
		STB,
		LODEPNG
	};

	template <>
	class ContentFactory<Texture> : public IContentFactory {
	private:
		std::unordered_map<std::string, TextureLoader> mExtensionToLoader;

		template <bool overrideFormat>
		Texture* loadGliInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		Texture* loadPngInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		Texture* loadStbInternal(const std::string& source,
			GLenum internalFormat);
		template <bool overrideFormat>
		Texture* loadInternal(const std::string& source,
			GLenum internalFormat);

	public:
		ContentFactory();
		~ContentFactory();

		INodeOwner* load(const std::string& source, Node loadInto) override;
		INodeOwner* loadEx(const std::string& source, Node loadInto, const void* extParams) override;
		
		Texture* loadGliUnmanaged(const std::string& source);
		Texture* loadPngUnmanaged(const std::string& source);
		Texture* loadStbUnmanaged(const std::string& source);
		Texture* loadGliUnmanaged(const std::string& source,
			GLenum internalFormat);
		Texture* loadPngUnmanaged(const std::string& source,
			GLenum internalFormat);
		Texture* loadStbUnmanaged(const std::string& source,
			GLenum internalFormat);
		void unload(INodeOwner* ref) override;

		Texture* loadTextureUnmanaged(const std::string& source);
		Texture* loadTextureUnmanaged(const std::string& source, 
			GLenum internalFormat);

		Texture* makeTexture2DUnmanaged(const uint width, const uint height, 
			const GLenum format, const int miplevels = -1);
		Texture* makeTexture2D(INodeOwner* parent, const uint width, 
			const uint height, const GLenum format, const int miplevels = -1);
		Texture* makeCubemapUnmanaged(const uint width, const uint height, 
			const GLenum format, const int miplevels = -1);
		Texture* makeCubemap(INodeOwner* parent, const uint width, 
			const uint height, const GLenum format, const int miplevels = -1);

		Texture* makeTexture2DMultisampleUnmanaged(const uint width, const uint height,
			const GLenum format, const uint samples);
		Texture* makeTexture2DMultisample(INodeOwner* parent, 
			const uint width, const uint height,
			const GLenum format, const uint samples);
	
		std::string getContentTypeString() const override;
	};
}