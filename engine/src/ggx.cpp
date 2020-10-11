#include <engine/ggx.hpp>

#define COMPUTE_KERNEL_MAX_TEXTURES 8

namespace Morpheus {
	GGXComputeKernel::GGXComputeKernel(uint groupSize) : INodeOwner(NodeType::GGX_COMPUTE_KERNEL),
		mGPUBackend(nullptr), mGroupSize(groupSize) {
	}

	void GGXComputeKernel::beginQueue() {
		mJobs.clear();
        bInJob = false;
	}

	Texture* GGXComputeKernel::addJobUnmanaged(const GGXComputeJob& job) {
		if (job.mInputImage->width() != job.mInputImage->height())
			throw std::runtime_error("GGXComputeKernel: Dimensions must be equal!");

		if ((job.mInputImage->width() & (job.mInputImage->width() - 1) == 0) || job.mInputImage->width() == 0)
			throw std::runtime_error("GGXComputeKernel: Dimensions are not a power of two!");

		if (!mGPUBackend)
            throw std::runtime_error("GGXComputeKernel: Kernel has not been initialized!");

        if (bInJob)
            throw std::runtime_error("GGXComputeKernel: Pending Jobs!");
        
        if (job.mInputImage->textureType() != TextureType::CUBE_MAP)
            throw std::runtime_error("GGXComputeKernel: Input must be cubemap!");

        uint result = mJobs.size();
        mJobs.emplace_back(job);

		Texture* tex = getFactory<Texture>()->makeCubemapUnmanaged(
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

				// Assign a compute job to every mip level individually, batch remaining mips together when
				// mips become small enough to fit in a single compute group.
				if (job.mInputImage->width() >> currentLevel <= mGroupSize) {
					for (; unit < COMPUTE_KERNEL_MAX_TEXTURES && currentLevel < job.mInputImage->levels(); ++currentLevel, ++unit) {
						glBindImageTexture(unit, outputTexture->id(), currentLevel, false, 0, 
							GL_WRITE_ONLY, outputTexture->format());
					}
				}
				else {
					glBindImageTexture(unit, outputTexture->id(), currentLevel, false, 0, 
						GL_WRITE_ONLY, outputTexture->format());
					unit = 1;
					++currentLevel;
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

	Texture* GGXComputeKernel::submit(const GGXComputeJob& job) {
		beginQueue();
		Texture* tex = addJobUnmanaged(job);
		submitQueue();
		return tex;
	}

	void GGXComputeKernel::barrier() {
		glTextureBarrier();
	}

	void GGXComputeKernel::init() {
		mCubemapSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, this);

		if (!mGPUBackend) {
			// Add compute shader to this object's children for resource management
			ContentExtParams<Shader> params;
			params.mConfigOverride.mDefines["GROUP_SIZE"] = std::to_string(mGroupSize);
			mGPUBackend = loadExt<Shader>("/internal/convggx.comp", params, this, true);
					
			mInputSamplerUniform.find(mGPUBackend, "inputTexture");
			
			if (!mInputSamplerUniform.valid())
				throw std::runtime_error("GGXComputeKernel: Could not find uniform inputTexture!");
			
			mOutputTextureCount.find(mGPUBackend, "outputTextureCount");

			if (!mOutputTextureCount.valid())
				throw std::runtime_error("GGXComputeKernel: Could not find uniform outputTextureCount!");
			
			mBeginLevel.find(mGPUBackend, "beginLevel");

			if (!mBeginLevel.valid())
				throw std::runtime_error("GGXComputeKernel: Could not find uniform beginLevel!");
			
			mTotalLevels.find(mGPUBackend, "totalLevels");

			if (!mTotalLevels.valid())
				throw std::runtime_error("GGXComputeKernel: Could not find uniform totalLevels!");
		}
	}
}