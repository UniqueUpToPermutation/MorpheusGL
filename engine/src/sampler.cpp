#include <engine/sampler.hpp>
#include <unordered_map>

namespace Morpheus {
	Sampler* Sampler::toSampler() {
		return this;
	}

	SamplerParameters makeSamplerParams(SamplerPrototype prototype) {
		SamplerParameters params;
		if (prototype == SamplerPrototype::TRILINEAR_CLAMP ||
			prototype == SamplerPrototype::TRILINEAR_TILE) {
			params.mMagFilter = GL_LINEAR;
			params.mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		}
		else if (prototype == SamplerPrototype::BILINEAR_CLAMP || 
			prototype == SamplerPrototype::BILINEAR_TILE) {
			params.mMagFilter = GL_LINEAR;
			params.mMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		}
		else if (prototype == SamplerPrototype::POINT_CLAMP ||
			prototype == SamplerPrototype::POINT_TILE) {
			params.mMagFilter = GL_NEAREST;
			params.mMinFilter = GL_NEAREST_MIPMAP_NEAREST;
		}

		if (prototype == SamplerPrototype::BILINEAR_CLAMP || 
			prototype == SamplerPrototype::TRILINEAR_CLAMP ||
			prototype == SamplerPrototype::POINT_CLAMP) {
			params.mWrapR = GL_CLAMP_TO_EDGE;
			params.mWrapS = GL_CLAMP_TO_EDGE;
			params.mWrapT = GL_CLAMP_TO_EDGE;
		}
		else if (prototype == SamplerPrototype::BILINEAR_TILE || 
			prototype == SamplerPrototype::TRILINEAR_TILE ||
			prototype == SamplerPrototype::POINT_TILE) {
			params.mWrapR = GL_REPEAT;
			params.mWrapS = GL_REPEAT;
			params.mWrapT = GL_REPEAT;
		}
		else if (prototype == SamplerPrototype::TRILINEAR_HDRI_SAMPLER) {
			params.mMagFilter = GL_LINEAR;
			params.mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			params.mWrapR = GL_REPEAT;
			params.mWrapS = GL_CLAMP_TO_EDGE;
			params.mWrapT = GL_CLAMP_TO_EDGE;
		}

		return params;
	}

	INodeOwner* ContentFactory<Sampler>::load(const std::string& source, Node loadInto)
	{
		std::cout << "Loading sampler " << source << "..." << std::endl;

		auto it = mStringToPrototypeMap.find(source);
		if (it != mStringToPrototypeMap.end()) {
			auto params = makeSamplerParams(it->second);
			return makeInternal(params);
		}
		else {
			std::cout << "Could not find sampler type " << source << ", defaulting to TRILINEAR_TILE!" << std::endl;
			auto params = makeSamplerParams(SamplerPrototype::TRILINEAR_TILE);
			return makeInternal(params);
		}
	}

	ContentFactory<Sampler>::ContentFactory() {
		mStringToPrototypeMap[TRILINEAR_CLAMP_SAMPLER_SRC] = SamplerPrototype::TRILINEAR_CLAMP;
		mStringToPrototypeMap[TRILINEAR_TILE_SAMPLER_SRC] = SamplerPrototype::TRILINEAR_TILE;
		mStringToPrototypeMap[BILINEAR_CLAMP_SAMPLER_SRC] = SamplerPrototype::BILINEAR_CLAMP;
		mStringToPrototypeMap[BILINEAR_TILE_SAMPLER_SRC] = SamplerPrototype::BILINEAR_TILE;
		mStringToPrototypeMap[POINT_CLAMP_SAMPLER_SRC] = SamplerPrototype::POINT_CLAMP;
		mStringToPrototypeMap[POINT_TILE_SAMPLER_SRC] = SamplerPrototype::POINT_TILE;
		mStringToPrototypeMap[TRILINEAR_HDRI_SAMPLER_SRC] = SamplerPrototype::TRILINEAR_HDRI_SAMPLER;
	}

	void ContentFactory<Sampler>::unload(INodeOwner* ref)
	{
		auto sampler = ref->toSampler();
		glDeleteSamplers(1, &sampler->mId);
		delete sampler;
	}
	
	ContentFactory<Sampler>::~ContentFactory() {
	}

	Sampler* ContentFactory<Sampler>::makeInternal(const SamplerParameters& params) {
		GLuint sampler_id;
		glCreateSamplers(1, &sampler_id);
		glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, params.mMinFilter);
		glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, params.mMagFilter);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, params.mWrapT);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, params.mWrapS);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_R, params.mWrapR);
		GL_ASSERT;
		auto sampler = new Sampler();
		sampler->mId = sampler_id;
		return sampler;
	}

	Sampler* ContentFactory<Sampler>::makeUnmanaged(const SamplerParameters& params) {
		return makeInternal(params);
	}

	Sampler* ContentFactory<Sampler>::makeUnmanaged(SamplerPrototype prototype) {
		return makeUnmanaged(makeSamplerParams(prototype));
	}

	std::string ContentFactory<Sampler>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(Sampler);
	}
}