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

		inline Texture* addJob(const HDRIToCubeComputeJob& job, INodeOwner* parent) {
			Texture* tex = addJobUnmananged(job);
			createContentNode(tex, parent);
			return tex;
		}

		inline Texture* submit(const HDRIToCubeComputeJob& job, INodeOwner* parent) {
			Texture* tex = submitUnmananged(job);
			createContentNode(tex, parent);
			return tex;
		}

		// Use this before attempting to access results of jobs
		void barrier();
	};
	SET_NODE_ENUM(HDRIToCubeKernel, CUSTOM_KERNEL);
}