#pragma once

#include "core.hpp"

namespace Morpheus {
	class Texture;
	class Shader;
	
	class LambertBRDFFilter {
	private:
		ref<Shader> mCompute;

	public:
		LambertBRDFFilter(ref<Shader> compute);

		void dispatch(Texture inputLightField, Texture outputLightField);
	};
}