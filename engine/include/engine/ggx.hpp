#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

#define GGX_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE 32

namespace Morpheus {
	struct GGXComputeJob {
		Texture* mInputImage;
	};

	class GGXComputeKernel : public INodeOwner {
	private:
		std::vector<GGXComputeJob> mJobs;
		std::vector<Texture*> mJobTextures;

		Sampler* mCubemapSampler;
		ShaderUniform<Sampler> mInputSamplerUniform;
		ShaderUniform<GLuint> mOutputTextureCount;
		ShaderUniform<GLuint> mBeginLevel;
		ShaderUniform<GLuint> mTotalLevels;

		Shader* mGPUBackend;

		bool bInJob;
		uint mGroupSize;

	public:

		GGXComputeKernel(uint groupSize = GGX_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE);

		void beginQueue();
		Texture* addJobUnmanaged(const GGXComputeJob& job);
		void submitQueue();

		Texture* submitUnmanaged(const GGXComputeJob& job);

		void barrier();

		void init() override;
	};
	SET_NODE_ENUM(GGXComputeKernel, GGX_COMPUTE_KERNEL);
}