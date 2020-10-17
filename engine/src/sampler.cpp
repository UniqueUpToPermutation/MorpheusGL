#include <engine/sampler.hpp>
#include <unordered_map>

namespace Morpheus {
	Sampler* Sampler::toSampler() {
		return this;
	}

	void Sampler::setMinFilter(MinFilter filter) {
		glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, (GLint)filter);
	}
	void Sampler::setMagFilter(MagFilter filter) {
		glSamplerParameteri(mId, GL_TEXTURE_MAG_FILTER, (GLint)filter);
	}
	void Sampler::setWrapS(TextureWrap wrap) {
		glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, (GLint)wrap);
	}
	void Sampler::setWrapT(TextureWrap wrap) {
		glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, (GLint)wrap);
	}
	void Sampler::setWrapR(TextureWrap wrap) {
		glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, (GLint)wrap);
	}
	void Sampler::setAnisotropy(float value) {
		glSamplerParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, value);
	}

 	ContentExtParams<Sampler> makeSamplerParams(SamplerPrototype prototype) {
		ContentExtParams<Sampler> params;
		if (prototype == SamplerPrototype::TRILINEAR_CLAMP ||
			prototype == SamplerPrototype::TRILINEAR_TILE) {
			params.mMagFilter = MagFilter::LINEAR;
			params.mMinFilter = MinFilter::LINEAR_MIPMAP_LINEAR;
		}
		else if (prototype == SamplerPrototype::BILINEAR_CLAMP || 
			prototype == SamplerPrototype::BILINEAR_TILE) {
			params.mMagFilter = MagFilter::LINEAR;
			params.mMinFilter = MinFilter::LINEAR_MIPMAP_NEAREST;
		}
		else if (prototype == SamplerPrototype::POINT_CLAMP ||
			prototype == SamplerPrototype::POINT_TILE) {
			params.mMagFilter = MagFilter::NEAREST;
			params.mMinFilter = MinFilter::NEAREST;
		}

		if (prototype == SamplerPrototype::BILINEAR_CLAMP || 
			prototype == SamplerPrototype::TRILINEAR_CLAMP ||
			prototype == SamplerPrototype::POINT_CLAMP) {
			params.mWrapR = TextureWrap::CLAMP_TO_EDGE;
			params.mWrapS = TextureWrap::CLAMP_TO_EDGE;
			params.mWrapT = TextureWrap::CLAMP_TO_EDGE;
		}
		else if (prototype == SamplerPrototype::BILINEAR_TILE || 
			prototype == SamplerPrototype::TRILINEAR_TILE ||
			prototype == SamplerPrototype::POINT_TILE) {
			params.mWrapR = TextureWrap::REPEAT;
			params.mWrapS = TextureWrap::REPEAT;
			params.mWrapT = TextureWrap::REPEAT;
		}
		else if (prototype == SamplerPrototype::TRILINEAR_HDRI_SAMPLER) {
			params.mMagFilter = MagFilter::LINEAR;
			params.mMinFilter = MinFilter::LINEAR_MIPMAP_LINEAR;
			params.mWrapR = TextureWrap::REPEAT;
			params.mWrapS = TextureWrap::CLAMP_TO_EDGE;
			params.mWrapT = TextureWrap::CLAMP_TO_EDGE;
		}

		params.mAnisotropy = 1.0f;

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

	INodeOwner* ContentFactory<Sampler>::loadEx(const std::string& source, 
		Node loadInto, const void* extParams) {
		return makeInternal(*reinterpret_cast<const ContentExtParams<Sampler>*>(extParams));
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

	Sampler* ContentFactory<Sampler>::makeInternal(const ContentExtParams<Sampler>& params) {
		GLuint sampler_id;
		glCreateSamplers(1, &sampler_id);
		glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, (GLint)params.mMinFilter);
		glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, (GLint)params.mMagFilter);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, (GLint)params.mWrapT);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, (GLint)params.mWrapS);
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_R, (GLint)params.mWrapR);
		glSamplerParameterf(sampler_id, GL_TEXTURE_MAX_ANISOTROPY, params.mAnisotropy);
		GL_ASSERT;
		auto sampler = new Sampler();
		sampler->mId = sampler_id;
		return sampler;
	}

	Sampler* ContentFactory<Sampler>::makeUnmanaged(const ContentExtParams<Sampler>& params) {
		return makeInternal(params);
	}

	Sampler* ContentFactory<Sampler>::makeUnmanaged(SamplerPrototype prototype) {
		return makeUnmanaged(makeSamplerParams(prototype));
	}

	std::string ContentFactory<Sampler>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(Sampler);
	}
}