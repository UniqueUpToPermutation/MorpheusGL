#include <engine/core.hpp>
#include <engine/shader.hpp>

#define HDRI_TO_CUBE_KERNEL_DEFAULT_GROUP_SIZE 32

namespace Morpheus {
	struct HDRIToCubeComputeJob {
		Texture* mHDRI;
		uint mTextureSize;
		GLenum mOutputFormat;
	};

	class HDRIToCubeKernel : public INodeOwner {
	private:
		Shader* mGPUBackend;
		Sampler* mSampler;
		ShaderUniform<Texture> mOutputTexture;
		ShaderUniform<Sampler> mInputSampler;

		bool bInJob;
		uint mGroupSize;

		std::vector<HDRIToCubeComputeJob> mJobs;
		std::vector<Texture*> mOutputTextures;

	public:
		HDRIToCubeKernel(uint groupSize = HDRI_TO_CUBE_KERNEL_DEFAULT_GROUP_SIZE);

		void init() override;

		// These functions will submit a batch of jobs to the GPU
		void beginQueue();
		Texture* addJobUnmananged(const HDRIToCubeComputeJob& job);
		void submitQueue();

		// This function submits a single job to the GPU
		Texture* submitUnmananged(const HDRIToCubeComputeJob& job);

		// Use this before attempting to access results of jobs
		void barrier();
	};
	SET_NODE_ENUM(HDRIToCubeKernel, CUSTOM_KERNEL);
}