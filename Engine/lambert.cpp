#include "lambert.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace Morpheus {
	LambertBRDFFilter::LambertBRDFFilter(ref<Shader> compute) : mCompute(compute) {
	}

	void LambertBRDFFilter::dispatch(Texture inputLightField, Texture outputLightField) {

	}
}