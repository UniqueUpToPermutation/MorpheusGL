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

#define MATERIAL_TEXTURE_2D_DEFAULT_SAMPLER_SRC TRILINEAR_TILE_SAMPLER_SRC
#define MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC TRILINEAR_CLAMP_SAMPLER_SRC

namespace Morpheus {
	class Sampler {
	private:
		GLenum mId;

	public:
		inline GLuint id() const { return mId; }

		inline void bind(GLuint unit) const {
			glBindSampler(unit, mId);
		}

		friend class ContentFactory<Sampler>;
	};
	SET_NODE_ENUM(Sampler, SAMPLER);

	struct SamplerParameters {
		GLint mMinFilter;
		GLint mMagFilter;
		GLint mWrapS;
		GLint mWrapT;
		GLint mWrapR;
	};

	enum class SamplerPrototype {
		TRILINEAR_CLAMP,
		TRILINEAR_TILE,
		BILINEAR_CLAMP,
		BILINEAR_TILE,
		POINT_CLAMP,
		POINT_TILE,
	};

	SamplerParameters makeSamplerParams(SamplerPrototype prototype);

	template <>
	class ContentFactory<Sampler> : public IContentFactory {
	private:
		ref<Sampler> makeInternal(const SamplerParameters& params);
		std::unordered_map<std::string, SamplerPrototype> mStringToPrototypeMap;

	public:
		ContentFactory();

		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		Node makeUnmanaged(const SamplerParameters& params, ref<Sampler>* samplerOut = nullptr);
		Node makeUnmanaged(const SamplerPrototype prototype, ref<Sampler>* samplerOut = nullptr);
	};
}