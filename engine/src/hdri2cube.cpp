#include <engine/hdri2cube.hpp>

namespace Morpheus {

	HDRIToCubeKernel::HDRIToCubeKernel(uint groupSize) : INodeOwner(NodeType::CUSTOM_KERNEL), 
		mGroupSize(groupSize), mGPUBackend(nullptr), bInJob(false) {

	}

	void HDRIToCubeKernel::init() {
		if (!mGPUBackend) {
			ContentExtParams<Shader> params;
			params.mConfigOverride.mDefines["GROUP_SIZE"] = std::to_string(mGroupSize);

			mGPUBackend = loadExt<Shader>("/internal/hdri2cube.comp", params, this);
			mSampler = load<Sampler>(TRILINEAR_HDRI_SAMPLER_SRC, this);

			mOutputTexture.find(mGPUBackend, "img_output");
			if (!mOutputTexture.valid()) 
				throw std::runtime_error("HDRIToCubeKernel: Could not find img_output uniform!");

			mInputSampler.find(mGPUBackend, "img_input");
			if (!mInputSampler.valid())
				throw std::runtime_error("HDRIToCubeKernel: Could not find img_input uniform!");
		}
	}

	void HDRIToCubeKernel::beginQueue() {
		mJobs.clear();
        bInJob = false;
	}

	Texture* HDRIToCubeKernel::addJobUnmananged(const HDRIToCubeComputeJob& job) {
		mJobs.emplace_back(job);
		Texture* tex = getFactory<Texture>()->makeCubemapUnmanaged(job.mTextureSize, job.mTextureSize, job.mOutputFormat);
		mOutputTextures.emplace_back(tex);
		return tex;
	}

	void HDRIToCubeKernel::submitQueue() {
		if (bInJob)
			throw std::runtime_error("LambertComputeKernel: Pending Jobs!");

		mGPUBackend->bind();

		for (int i = 0; i < mJobs.size(); ++i) {
			auto& job = mJobs[i];
			auto tex = mOutputTextures[i];

			mInputSampler.set(job.mHDRI, mSampler);
			mOutputTexture.set(tex, GL_WRITE_ONLY);

			uint groups_x = tex->width() / mGroupSize;
			uint groups_y = tex->height() / mGroupSize;

			glDispatchCompute(groups_x, groups_y, 6);
		}
	}

	Texture* HDRIToCubeKernel::submitUnmananged(const HDRIToCubeComputeJob& job) {
		beginQueue();
		Texture* tex = addJobUnmananged(job);
		submitQueue();
		return tex;
	}

	void HDRIToCubeKernel::barrier() {
		glTextureBarrier();

		for (auto& tex : mOutputTextures) {
			tex->genMipmaps();
		}
	}
}