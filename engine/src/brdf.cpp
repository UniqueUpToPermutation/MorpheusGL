#include <engine/brdf.hpp>

namespace Morpheus {
	CookTorranceLUTComputeKernel::CookTorranceLUTComputeKernel(ref<Shader> gpuBackend, 
		uint computeGroupSize) : 
		mGPUBackend(gpuBackend), 
		mGroupSize(computeGroupSize) {
	}

	ref<Texture> CookTorranceLUTComputeKernel::submit(uint roughnessPixels, uint NoVPixels) {
		auto result = getFactory<Texture>()->makeTexture2DUnmanaged(roughnessPixels, NoVPixels, GL_RG16, 1);

		uint groups_x = result->width() / mGroupSize;
		uint groups_y = result->height() / mGroupSize;

		glUseProgram(mGPUBackend->id());
		glBindImageTexture(0, result->id(), 0, false, 0, GL_WRITE_ONLY, GL_RG16);
		glDispatchCompute(groups_x, groups_y, 1);

		return result;
	}

	void CookTorranceLUTComputeKernel::barrier() {
		glTextureBarrier();
	}

	void CookTorranceLUTComputeKernel::init(Node node) {
	}

	void CookTorranceLUTComputeKernel::dispose() {
		delete this;
	}
}