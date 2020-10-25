#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>
#include <engine/lambert.hpp>

#define LAMBERT_SH_COEFFS 9
#define LAMBERT_SH_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE 32

namespace Morpheus {

	struct LambertSHComputeJob {
		Texture* mInputImage;

		inline LambertSHComputeJob(Texture* image = nullptr) : mInputImage(image) {
		}
	};

	// Implements a lambert convolution in the SH basis on the GPU. Returns a set of 9 SH coefficients for 3 color channels.
	class LambertSHComputeKernel : public INodeOwner {
	private:
		GLuint mGPUOutputBuffer;
		
		std::vector<LambertSHComputeJob> mJobs;
		std::vector<float> mResultBuffer;

		Sampler* mCubemapPointSampler;

		Shader* mGPUBackend;
		ShaderUniform<uint> mOffsetUniform;
		ShaderUniform<glm::ivec2> mImageSizeUniform;
		ShaderUniform<Sampler> mInputImage;

		LambertSHKernelCPU mSHTransferFunction;

		bool bInJob;
		uint mGroupSize;

	public:

		LambertSHComputeKernel(uint groupSize = LAMBERT_SH_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE);
		~LambertSHComputeKernel();

		void init() override;

		// These functions will submit a batch of jobs to the GPU
		void beginQueue();
		uint addJob(const LambertSHComputeJob& job);
		void submitQueue();

		// This function submits a single job to the GPU
		void submit(const LambertSHComputeJob& job);

		// Use this before attempting to access results of jobs
		void barrier();

		float* results(uint job_id = 0);

		inline uint shCount() const {
			return LAMBERT_SH_COEFFS;
		}
	};
	SET_NODE_ENUM(LambertSHComputeKernel, LAMBERT_SH_COMPUTE_KERNEL);
}