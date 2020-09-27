#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

#define DEFAULT_CT_LUT_PIXELS 128
#define DEFAULT_CT_LUT_GROUP_SIZE 32

namespace Morpheus {
	class CookTorranceLUTComputeKernel : public INodeOwner {
	private:
		Shader* mGPUBackend;

		uint mGroupSize;

	public:
		CookTorranceLUTComputeKernel(uint computeGroupSize = DEFAULT_CT_LUT_GROUP_SIZE);

		Texture* submit(uint roughnessPixels = DEFAULT_CT_LUT_PIXELS, 
			uint NoVPixels = DEFAULT_CT_LUT_PIXELS);

		void barrier();

		void init() override;
	};
	SET_NODE_ENUM(CookTorranceLUTComputeKernel, COOK_TORRANCE_LUT_COMPUTE_KERNEL);
}