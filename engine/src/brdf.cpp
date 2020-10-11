#include <engine/brdf.hpp>

namespace Morpheus {
	CookTorranceLUTComputeKernel::CookTorranceLUTComputeKernel(uint computeGroupSize) : 
		INodeOwner(NodeType::COOK_TORRANCE_LUT_COMPUTE_KERNEL),
		mGPUBackend(nullptr), 
		mGroupSize(computeGroupSize) {
	}

	Texture* CookTorranceLUTComputeKernel::submit(uint roughnessPixels, uint NoVPixels) {
		if (!mGPUBackend)
			std::runtime_error("CookTorranceLUTComputeKernel has not been initialized yet!");

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

	void CookTorranceLUTComputeKernel::init() {
		if (!mGPUBackend) {
			// Set configuration and load shader, and set as child
			ContentExtParams<Shader> params;
			params.mConfigOverride.mDefines["GROUP_SIZE"] = std::to_string(mGroupSize);
			mGPUBackend = loadExt<Shader>("/internal/cooktorrancelut.comp", params, this, true);
		}	
	}
}