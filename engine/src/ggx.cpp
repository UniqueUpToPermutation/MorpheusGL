#include <engine/ggx.hpp>

#define COMPUTE_KERNEL_MAX_TEXTURES 8

namespace Morpheus {

	GGXComputeKernel::GGXComputeKernel(uint groupSize) : INodeOwner(NodeType::GGX_COMPUTE_KERNEL),
		mGPUBackend(nullptr), mGroupSize(groupSize), bInJob(false) {
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

			mInputSamplerUniform.set(job.mInputImage, mCubemapSampler);

			const float deltaRoughness = 1.0f / glm::max(float(outputTexture->levels() - 1), 1.0f);

			// Copy 0th mipmap level into destination environment map.
			glCopyImageSubData(job.mInputImage->id(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				outputTexture->id(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				outputTexture->width(), outputTexture->height(), 6);

			GL_ASSERT;

			for (uint level = 1; level < outputTexture->levels(); ++level) {
				float roughness = deltaRoughness * level;

				mRoughness.set(roughness);
				mOutputTextureImage.set(outputTexture, GL_WRITE_ONLY, level);

				uint num_groups = (job.mInputImage->width() >> level) / mGroupSize;
				num_groups = glm::max(num_groups, 1u);
				glDispatchCompute(num_groups, num_groups, 6);

				GL_ASSERT;
			}
        }

        bInJob = true;
	}

	Texture* GGXComputeKernel::submitUnmanaged(const GGXComputeJob& job) {
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
			mGPUBackend = loadEx<Shader>("/internal/convggx.comp", params, this, true);
					
			mInputSamplerUniform.find(mGPUBackend, "inputTexture");
			
			if (!mInputSamplerUniform.valid())
				throw std::runtime_error("GGXComputeKernelOld: Could not find uniform inputTexture!");
			
			mOutputTextureImage.find(mGPUBackend, "outputTexture");

			if (!mOutputTextureImage.valid())
				throw std::runtime_error("GGXComputeKernelOld: Could not find uniform outputTexture!");

			mRoughness.find(mGPUBackend, "roughness");

			if (!mRoughness.valid())
				throw std::runtime_error("GGXComputeKernel: Could not find uniform roughness!");
		}
	}
}