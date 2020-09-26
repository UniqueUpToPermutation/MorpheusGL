#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

namespace Morpheus {
	struct GGXComputeJob {
		ref<Texture> mInputImage;
	};

	class GGXComputeKernel : public IInitializable, public IDisposable {
	private:
		std::vector<GGXComputeJob> mJobs;
		std::vector<ref<Texture>> mJobTextures;

		ref<Sampler> mCubemapSampler;
		ShaderUniform<Sampler> mInputSamplerUniform;
		ShaderUniform<GLuint> mOutputTextureCount;
		ShaderUniform<GLuint> mBeginLevel;
		ShaderUniform<GLuint> mTotalLevels;

		ref<Shader> mGPUBackend;
		NodeHandle mGPUBackendHandle;

		bool bInJob;
		uint mGroupSize;

	public:

		GGXComputeKernel(ref<Shader> gpuBackend, uint groupSize);

		void beginQueue();
		ref<Texture> addJobUnmanaged(const GGXComputeJob& job);
		void submitQueue();

		ref<Texture> submit(const GGXComputeJob& job);

		void barrier();

		void init(Node node) override;
		void dispose() override;
	};
	SET_NODE_ENUM(GGXComputeKernel, GGX_COMPUTE_KERNEL);
}