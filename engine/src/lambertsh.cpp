#include <engine/lambertsh.hpp>

namespace Morpheus {
  	auto outputFloatSizeFor(const Texture* image, const uint mGroupSize) {
        return sizeof(float) * LAMBERT_SH_COEFFS *
            image->height() / mGroupSize;
    };

	LambertSHComputeKernel::LambertSHComputeKernel(uint groupSize) : INodeOwner(NodeType::LAMBERT_SH_COMPUTE_KERNEL),
        mGroupSize(groupSize), mGPUBackend(nullptr), bInJob(false) {
    }

	void LambertSHComputeKernel::submit(const LambertSHComputeJob& job) {
        beginQueue();
        addJob(job);
        submitQueue();
    }

    void LambertSHComputeKernel::submitQueue() {
        if (bInJob)
            throw std::runtime_error("LambertSHComputeKernel: Pending Jobs!");

        uint bufferSize = 0;
        for (auto& job : mJobs) {
            bufferSize += sizeof(float) * outputFloatSizeFor(job.mInputImage, mGroupSize);
        }

        glCreateBuffers(1, &mGPUOutputBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mGPUOutputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STREAM_READ);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

        glUseProgram(mGPUBackend->id());

        GL_ASSERT;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mGPUOutputBuffer);

        GL_ASSERT;

        uint offset = 0;
        for (auto& job : mJobs) {
            uint num_groups_x = job.mInputImage->height() / mGroupSize;

            mOffsetUniform.set(offset);
			mInputImage.set(job.mInputImage, mCubemapPointSampler);
			mImageSizeUniform.set(job.mInputImage->size2d());

            GL_ASSERT;

            glDispatchCompute(num_groups_x, 1, 1);

            offset += outputFloatSizeFor(job.mInputImage, mGroupSize);
        }

        bInJob = true;
    }

    float* LambertSHComputeKernel::results(uint job_id) {
        uint blockSize = 3 * LAMBERT_SH_COEFFS;
        return &mResultBuffer[blockSize * job_id];
    }

    void LambertSHComputeKernel::barrier() {

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mGPUOutputBuffer);
        void* buf_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        float* out_data = static_cast<float*>(buf_data);

        mResultBuffer.resize(mJobs.size() * 3 * LAMBERT_SH_COEFFS);
        std::fill(mResultBuffer.begin(), mResultBuffer.end(), 0.0f);

        // Sum over work group results on the CPU
        uint offset = 0;
        uint blockSize = 3 * LAMBERT_SH_COEFFS;
        for (uint i_job = 0; i_job < mJobs.size(); ++i_job) {
            auto& job = mJobs[i_job];
            uint num_groups_x = job.mInputImage->height() / mGroupSize;

            for (uint group_result = 0; group_result < num_groups_x; ++group_result) {
                for (uint block_offset = 0; block_offset < blockSize; ++block_offset) {
                    mResultBuffer[i_job * blockSize + block_offset] += 
                        out_data[offset + group_result * blockSize + block_offset];
                }
            }

            offset += outputFloatSizeFor(job.mInputImage, mGroupSize);
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &mGPUOutputBuffer);
        mGPUOutputBuffer = 0;

        // Apply the transfer function to results
        for (uint i_job = 0; i_job < mJobs.size(); ++i_job) {
            mSHTransferFunction.applySH(&mResultBuffer[i_job * blockSize],
                &mResultBuffer[i_job * blockSize], LAMBERT_SH_COEFFS, 3);
        }
    }

    void LambertSHComputeKernel::beginQueue() {
        mJobs.clear();
        mResultBuffer.clear();
        bInJob = false;
    }

	 void LambertSHComputeKernel::init() {
		mCubemapPointSampler = load<Sampler>(POINT_CLAMP_SAMPLER_SRC);

        if (!mGPUBackend) {
			ContentExtParams<Shader> params;
			params.mConfigOverride.mDefines["GROUP_SIZE"] = std::to_string(mGroupSize);
			mGPUBackend = loadEx<Shader>("/internal/lambertsh.comp", params, this, true);

            mOffsetUniform.find(mGPUBackend, "outputOffset");

			if (!mOffsetUniform.valid()) {
				throw std::runtime_error("LambertSHComputeKernel: could not find outputOffset!");
			}

			mImageSizeUniform.find(mGPUBackend, "imageSize");

			if (!mImageSizeUniform.valid()) {
				throw std::runtime_error("LambertSHComputeKernel: could not find imageSize!");
			}

			mInputImage.find(mGPUBackend, "img_input");

			if (!mInputImage.valid()) {
				throw std::runtime_error("LambertSHComputeKernel: could not find img_input!");
			}

            bInJob = false;
            mGPUOutputBuffer = 0;
        }
    }

    LambertSHComputeKernel::~LambertSHComputeKernel() {
        if (mGPUOutputBuffer != 0) {
            glDeleteBuffers(1, &mGPUOutputBuffer);
        }
    }

    uint LambertSHComputeKernel::addJob(const LambertSHComputeJob& job) {
        if (!mGPUBackend)
            throw std::runtime_error("LambertSHComputeKernel: Kernel has not been initialized!");

        if (bInJob)
            throw std::runtime_error("LambertSHComputeKernel: Pending Jobs!");
        
        if (job.mInputImage->textureType() != TextureType::CUBE_MAP)
            throw std::runtime_error("LambertSHComputeKernel: Input must be cubemap!");

        uint result = mJobs.size();
        mJobs.emplace_back(job);
        return result;
    }
}