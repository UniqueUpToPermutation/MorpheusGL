#include <engine/lambert.hpp>

#include <GLFW/glfw3.h>

namespace Morpheus {
	LambertComputeKernel::LambertComputeKernel(uint groupSize) :
		INodeOwner(NodeType::LAMBERT_COMPUTE_KERNEL), 
		mGroupSize(groupSize), mGPUBackend(nullptr) {
	}

	void LambertComputeKernel::init() {
		mCubemapSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, this);

		if (!mGPUBackend) {
			ContentExtParams<Shader> params;
			params.mConfigOverride.mDefines["GROUP_SIZE"] = std::to_string(mGroupSize);
			mGPUBackend = loadEx<Shader>("/internal/lambert.comp", params, this, true);

			mImageInput.find(mGPUBackend, "img_input");
			if (!mImageInput.valid()) {
				throw std::runtime_error("LambertComputeKernel: Image input not found!");
			}

			mImageOutput.find(mGPUBackend, "img_output");
			if (!mImageOutput.valid()) {
				throw std::runtime_error("LambertComputeKernel: Image output not found!");
			}

            bInJob = false;
        }
	}

	// These functions will submit a batch of jobs to the GPU
	void LambertComputeKernel::beginQueue() {
 		mJobs.clear();
        bInJob = false;
	}

	Texture* LambertComputeKernel::addJobUnmanaged(const LambertComputeJob& job) {
		mJobs.emplace_back(job);

		int size = job.mOutputSize;
		GLenum format = job.mOutputFormat;

		if (size <= 0) {
			size = job.mInputImage->width();
		}

		if (format <= 0) {
			format = job.mInputImage->format();
		}

		Texture* tex = getFactory<Texture>()->makeCubemapUnmanaged(
			size, size, format);

		mResultTextures.emplace_back(tex);
		return tex;
	}

	void LambertComputeKernel::submitQueue() {
        if (bInJob)
            throw std::runtime_error("GGXComputeKernel: Pending Jobs!");

        mGPUBackend->bind();

        GL_ASSERT;

        uint offset = 0;
        for (uint i_job = 0; i_job < mJobs.size(); ++i_job) {
			auto& job = mJobs[i_job];
			auto outputTexture = mResultTextures[i_job];
			auto inputTexture = job.mInputImage;

			mImageInput.set(inputTexture, mCubemapSampler);
			mImageOutput.set(outputTexture, GL_WRITE_ONLY);

			uint num_groups = outputTexture->width() / mGroupSize;
			num_groups = glm::max(num_groups, 1u);

			glDispatchCompute(num_groups, num_groups, 6);

			GL_ASSERT;
        }

        bInJob = true;
	}

	// This function submits a single job to the GPU
	Texture* LambertComputeKernel::submitUnmanaged(const LambertComputeJob& job) {
		beginQueue();
		Texture* tex = addJobUnmanaged(job);
		submitQueue();
		return tex;
	}

	// Use this before attempting to access results of jobs
	void LambertComputeKernel::barrier() {
		glTextureBarrier();

		for (auto tex : mResultTextures) {
			tex->genMipmaps();
			tex->savepng("test.png");
		}
	}
}