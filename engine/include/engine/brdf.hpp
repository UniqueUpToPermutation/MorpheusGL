#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

#define DEFAULT_CT_LUT_PIXELS 128
#define DEFAULT_CT_LUT_GROUP_SIZE 64

namespace Morpheus {
	class CookTorranceLUTComputeKernel : public IInitializable, public IDisposable {
	private:
		ref<Shader> mGPUBackend;
		NodeHandle mGPUBackendHandle;

		uint mGroupSize;

	public:
		CookTorranceLUTComputeKernel(ref<Shader> gpuBackend, 
			uint computeGroupSize = DEFAULT_CT_LUT_GROUP_SIZE);

		ref<Texture> submit(uint roughnessPixels = DEFAULT_CT_LUT_PIXELS, 
			uint NoVPixels = DEFAULT_CT_LUT_PIXELS);

		void barrier();

		void init(Node node) override;
		void dispose() override;
	};
	SET_NODE_ENUM(CookTorranceLUTComputeKernel, COOK_TORRANCE_LUT_COMPUTE_KERNEL);
}