#include "lambert.hpp"

namespace Morpheus {
	LambertBRDFFilter::LambertBRDFFilter(ref<Shader> compute) : mCompute(compute) {
	}

	void LambertBRDFFilter::dispatch(Texture inputLightField, Texture outputLightField) {

	}
}