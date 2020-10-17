#pragma once

#include <glad/glad.h>
#include <iostream>

#include <engine/content.hpp>

#define POINT_CLAMP_SAMPLER_SRC "POINT_CLAMP_SAMPLER"
#define POINT_TILE_SAMPLER_SRC "POINT_TILE_SAMPLER"
#define TRILINEAR_CLAMP_SAMPLER_SRC "TRILINEAR_CLAMP_SAMPLER"
#define TRILINEAR_TILE_SAMPLER_SRC "TRILINEAR_TILE_SAMPLER"
#define BILINEAR_CLAMP_SAMPLER_SRC "BILINEAR_CLAMP_SAMPLER"
#define BILINEAR_TILE_SAMPLER_SRC "BILINEAR_TILE_SAMPLER"
#define TRILINEAR_HDRI_SAMPLER_SRC "TRILINEAR_HDRI_SAMPLER"

#define RENDERER_TEXTURE_SAMPLER_SRC "RENDERER_TEXTURE_SAMPLER"
#define RENDERER_CUBEMAP_SAMPLER_SRC "RENDERER_CUBEMAP_SAMPLER"

#define MATERIAL_TEXTURE_2D_DEFAULT_SAMPLER_SRC RENDERER_TEXTURE_SAMPLER_SRC
#define MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC RENDERER_CUBEMAP_SAMPLER_SRC

namespace Morpheus {

	enum class MinFilter : GLint {
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR,
		NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
	};

	enum class MagFilter : GLint {
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR
	};

	enum class TextureWrap : GLint {
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
		REPEAT = GL_REPEAT
	};

	class Sampler : public INodeOwner {
	private:
		GLenum mId;

	public:
		inline Sampler() : INodeOwner(NodeType::SAMPLER) { }

		Sampler* toSampler() override;

		inline GLuint id() const { return mId; }

		void setMinFilter(MinFilter filter);
		void setMagFilter(MagFilter filter);
		void setWrapS(TextureWrap wrap);
		void setWrapT(TextureWrap wrap);
		void setWrapR(TextureWrap wrap);
		void setAnisotropy(float value);

		inline void bind(GLuint unit) const {
			glBindSampler(unit, mId);
		}

		friend class ContentFactory<Sampler>;
	};
	SET_NODE_ENUM(Sampler, SAMPLER);

	template <>
	struct ContentExtParams<Sampler> {
		MinFilter mMinFilter;
		MagFilter mMagFilter;
		TextureWrap mWrapS;
		TextureWrap mWrapT;
		TextureWrap mWrapR;
		float mAnisotropy;
	};

	enum class SamplerPrototype {
		TRILINEAR_CLAMP,
		TRILINEAR_TILE,
		BILINEAR_CLAMP,
		BILINEAR_TILE,
		POINT_CLAMP,
		POINT_TILE,
		TRILINEAR_HDRI_SAMPLER
	};

	ContentExtParams<Sampler> makeSamplerParams(SamplerPrototype prototype);

	template <>
	class ContentFactory<Sampler> : public IContentFactory {
	private:
		Sampler* makeInternal(const ContentExtParams<Sampler>& params);
		std::unordered_map<std::string, SamplerPrototype> mStringToPrototypeMap;

	public:
		ContentFactory();
		~ContentFactory();

		INodeOwner* load(const std::string& source, Node loadInto) override;

		// Creates a sampler with the desired properties
		INodeOwner* loadEx(const std::string& source, Node loadInto, const void* extParams) override;

		void unload(INodeOwner* ref) override;

		Sampler* makeUnmanaged(const ContentExtParams<Sampler>& params);
		Sampler* makeUnmanaged(const SamplerPrototype prototype);
	
		std::string getContentTypeString() const override;
	};
}