#include <engine/ggx.hpp>

#define COMPUTE_KERNEL_MAX_TEXTURES 8

namespace Morpheus {
	GGXComputeKernel::GGXComputeKernel(ref<Shader> gpuBackend, uint groupSize) :
		mGPUBackend(gpuBackend), mGroupSize(groupSize) {
		
		mInputSamplerUniform.find(gpuBackend, "inputTexture");
		
		if (mInputSamplerUniform.location() == -1)
			throw std::runtime_error("GGXComputeKernel: Could not find uniform inputTexture!");
		
		mOutputTextureCount.find(gpuBackend, "outputTextureCount");

		if (mOutputTextureCount.location() == -1)
			throw std::runtime_error("GGXComputeKernel: Could not find uniform outputTextureCount!");
		
		mBeginLevel.find(gpuBackend, "beginLevel");

		if (mBeginLevel.location() == -1)
			throw std::runtime_error("GGXComputeKernel: Could not find uniform beginLevel!");
		
		mTotalLevels.find(gpuBackend, "totalLevels");

		if (mTotalLevels.location() == -1)
			throw std::runtime_error("GGXComputeKernel: Could not find uniform totalLevels!");
	}

	void GGXComputeKernel::beginQueue() {
		mJobs.clear();
        bInJob = false;
	}

	ref<Texture> GGXComputeKernel::addJobUnmanaged(const GGXComputeJob& job) {
		if (job.mInputImage->width() != job.mInputImage->height())
			throw std::runtime_error("GGXComputeKernel: Dimensions must be equal!");

		if ((job.mInputImage->width() & (job.mInputImage->width() - 1) == 0) || job.mInputImage->width() == 0)
			throw std::runtime_error("GGXComputeKernel: Dimensions are not a power of two!");

		if (mGPUBackend.isNull())
            throw std::runtime_error("GGXComputeKernel: Kernel has not been initialized!");

        if (bInJob)
            throw std::runtime_error("GGXComputeKernel: Pending Jobs!");
        
        if (job.mInputImage->type() != TextureType::CUBE_MAP)
            throw std::runtime_error("GGXComputeKernel: Input must be cubemap!");

        uint result = mJobs.size();
        mJobs.emplace_back(job);

		ref<Texture> tex = getFactory<Texture>()->makeCubemapUnmanaged(
			job.mInputImage->width(), 
			job.mInputImage->height(), 
			job.mInputImage->format());

		mJobTextures.emplace_back(tex);

		return tex;
	}

	void GGXComputeKernel::submitQueue() {
 		if (bInJob)
            throw std::runtime_error("GGXComputeKernel: Pending Jobs!");

        glUseProgram(mGPUBackend->id());

        GL_ASSERT;

        uint offset = 0;
        for (uint i_job = 0; i_job < mJobs.size(); ++i_job) {
			auto& job = mJobs[i_job];
			auto outputTexture = mJobTextures[i_job];

			glBindTexture(GL_TEXTURE_CUBE_MAP, outputTexture->id());

			GL_ASSERT;

			// Bind sampler
			mTotalLevels.set(job.mInputImage->levels());

			GL_ASSERT;

			mInputSamplerUniform.set(job.mInputImage, mCubemapSampler);

			GL_ASSERT;

			for (uint currentLevel = 0; currentLevel < job.mInputImage->levels();) {
				uint beginLevel = currentLevel;
				uint unit = 0;

				GL_ASSERT;

				// Batch mipmaps into sets of 8
				for (; unit < COMPUTE_KERNEL_MAX_TEXTURES && currentLevel < job.mInputImage->levels(); ++currentLevel, ++unit) {
					glBindImageTexture(unit, outputTexture->id(), currentLevel, false, 0, 
						GL_WRITE_ONLY, outputTexture->format());
				}

				GL_ASSERT;
				
				mOutputTextureCount.set(unit);
				mBeginLevel.set(beginLevel);

				uint num_groups = job.mInputImage->width() >> beginLevel;

				GL_ASSERT;

				glDispatchCompute(num_groups, num_groups, 6);

				GL_ASSERT;
			}
        }

        bInJob = true;
	}

	ref<Texture> GGXComputeKernel::submit(const GGXComputeJob& job) {
		beginQueue();
		ref<Texture> tex = addJobUnmanaged(job);
		submitQueue();
		return tex;
	}

	void GGXComputeKernel::sync() {
		glTextureBarrier();
	}

	void GGXComputeKernel::init(Node node) {
		load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, node, &mCubemapSampler);
	}

	void GGXComputeKernel::dispose() {
		delete this;
	}
}