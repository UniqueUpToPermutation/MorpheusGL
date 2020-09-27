#include <engine/lambert.hpp>

#include <GLFW/glfw3.h>

static const char* computeKernelSource = 
R"(
    #define SH_C_0 0.28209479177
    #define SH_C_1 0.4886025119
    #define SH_C_2n2 1.09254843059
    #define SH_C_2n1 SH_C_2n2
    #define SH_C_20 0.31539156525
    #define SH_C_21 SH_C_2n2
    #define SH_C_22 0.54627421529

    #define FACE_POSITIVE_X 0
    #define FACE_NEGATIVE_X 1
    #define FACE_POSITIVE_Y 2
    #define FACE_NEGATIVE_Y 3
    #define FACE_POSITIVE_Z 4
    #define FACE_NEGATIVE_Z 5

    #define SH_COEFFS 9

    precision highp int;
    precision highp float;

    layout(local_size_x = GROUP_SIZE_X, local_size_y = 1, local_size_z = 1) in;
    layout(rgba8, binding = 0) uniform readonly imageCube img_input;

    layout(std430, binding = 0) writeonly buffer out_buffer {
        float out_array[];
    };

    uniform uint outputOffset = 0;

    float jacobian(vec2 surfacePosition) {
        float mag2 = dot(surfacePosition, surfacePosition) + 1.0;
        float mag = sqrt(mag2);
        return 1.0 / (mag2 * mag);
    }

    vec3 getDirection(vec2 surfacePosition, uint face) {
        vec3 result;
        switch (face) {
            case FACE_POSITIVE_X:
                result.x = 1.0;
                result.z = -surfacePosition.x;
                result.y = -surfacePosition.y;
                break;
            case FACE_NEGATIVE_X:
                result.x = -1.0;
                result.z = surfacePosition.x;
                result.y = -surfacePosition.y;
                break;
            case FACE_POSITIVE_Y:
                result.x = surfacePosition.x;
                result.y = 1.0;
                result.z = surfacePosition.y;
                break;
            case FACE_NEGATIVE_Y:
                result.x = surfacePosition.x;
                result.y = -1.0;
                result.z = -surfacePosition.y;
                break;
            case FACE_POSITIVE_Z:
                result.x = surfacePosition.x;
                result.y = -surfacePosition.y;
                result.z = 1.0;
                break;
            case FACE_NEGATIVE_Z:
                result.x = -surfacePosition.x;
                result.y = -surfacePosition.y;
                result.z = -1.0;
                break;
        }
        return normalize(result);
    }

    shared vec4 sum_array[gl_WorkGroupSize.x * SH_COEFFS];

    void main() {
        uint y_indx = gl_GlobalInvocationID.x;

        vec4 result[SH_COEFFS];

        // Clear SH_COEFFS
        for (uint i = 0; i < SH_COEFFS; ++i) {
            result[i] = vec4(0.0, 0.0, 0.0, 0.0);
        }

        ivec2 size = imageSize(img_input);

        float h2 = 4.0 / float(size.x) / float(size.y);
        vec2 uv = vec2(-1.0, -1.0 + 2.0 * (float(y_indx) + 0.5) / float(size.y));

        for (uint z_indx = 0; z_indx < 6; ++z_indx) {
            for (uint x_indx = 0; x_indx < size.x; ++x_indx) {
                uv.x = -1.0 + 2.0 * (float(x_indx) + 0.5) / float(size.x);

                vec4 pix = imageLoad(img_input, ivec3(x_indx, y_indx, z_indx));
                vec3 position = getDirection(uv, z_indx);
                float jac = jacobian(uv);

                // Integrate with the spherical harmonics
                float sh[SH_COEFFS];
                sh[0] = SH_C_0;
                sh[1] = SH_C_1 * position.y;
                sh[2] = SH_C_1 * position.z;
                sh[3] = SH_C_1 * position.x;
                sh[4] = SH_C_2n2 * position.x * position.y;
                sh[5] = SH_C_2n1 * position.y * position.z;
                sh[6] = SH_C_20 * (-position.x * position.x - 
                    position.y * position.y + 2.0 * position.z * position.z);
                sh[7] = SH_C_21 * position.z * position.x;
                sh[8] = SH_C_22 * (position.x * position.x - 
                    position.y * position.y);

                for (uint i = 0; i < SH_COEFFS; ++i) {
                    result[i] += pix * sh[i] * jac * h2;
                }
            }
        }

        for (uint i = 0; i < SH_COEFFS; ++i) {
            sum_array[SH_COEFFS * gl_LocalInvocationIndex + i] = result[i];
        }

        memoryBarrierShared();
        barrier();

        if (gl_LocalInvocationIndex < SH_COEFFS) {
            uint sh = gl_LocalInvocationIndex;
            vec4 final_result = vec4(0.0, 0.0, 0.0, 0.0);
            for (uint i = 0; i < gl_WorkGroupSize.x; ++i) {
                final_result += sum_array[i * SH_COEFFS + sh];
            }

            uint writePos = outputOffset + 3 * (gl_WorkGroupID.x * SH_COEFFS + sh);
            out_array[writePos] = final_result.x;
            out_array[writePos + 1] = final_result.y;
            out_array[writePos + 2] = final_result.z;
        }
    }
)";

namespace Morpheus {
    LambertComputeKernel::LambertComputeKernel(uint groupSize) : INodeOwner(NodeType::LAMBERT_COMPUTE_KERNEL),
        mGroupSize(groupSize), mGPUBackend(nullptr) {
    }

    void LambertComputeKernel::init() {
        if (!mGPUBackend) {
            std::string source(computeKernelSource);

            std::stringstream ss;
            ss << "#version 450" << std::endl;
            ss << "#define GROUP_SIZE_X " << mGroupSize << std::endl;
            ss << source;

            auto program = compileComputeKernel(ss.str());
                
            if (program == 0) {
                std::cout << "LambertComputeKernel: Error occured while compiling compute shader..." << std::endl;
                throw std::runtime_error("LambertComputeKernel: Compute shader failed to compile!");
            }

            // Add compute shader to this object's children for resource management
            mGPUBackend = getFactory<Shader>()->makeFromGL(program);
			createContentNode(mGPUBackend);
            addChild(mGPUBackend);

            mOffsetUniform.find(mGPUBackend, "outputOffset");

			if (!mOffsetUniform.valid()) {
				throw std::runtime_error("Lambert Computer Kernel: could not find outputOffset!");
			}

            bInJob = false;
            mGPUOutputBuffer = 0;
        }
    }

    LambertComputeKernel::~LambertComputeKernel() {
        if (mGPUOutputBuffer != 0) {
            glDeleteBuffers(1, &mGPUOutputBuffer);
        }
    }

    uint LambertComputeKernel::addJob(const LambertComputeJob& job) {
        if (!mGPUBackend)
            throw std::runtime_error("LambertComputeKernel: Kernel has not been initialized!");

        if (bInJob)
            throw std::runtime_error("LambertComputeKernel: Pending Jobs!");
        
        if (job.mInputImage->type() != TextureType::CUBE_MAP)
            throw std::runtime_error("LambertComputeKernel: Input must be cubemap!");

        if (job.mInputImage->format() != GL_RGBA8) 
            throw std::runtime_error("Image format not supported!");

        uint result = mJobs.size();
        mJobs.emplace_back(job);
        return result;
    }

    auto outputFloatSizeFor(const Texture* image, const uint mGroupSize) {
        return sizeof(float) * LAMBERT_SH_COEFFS *
            image->height() / mGroupSize;
    };

    void LambertComputeKernel::submit(const LambertComputeJob& job) {
        beginQueue();
        addJob(job);
        submitQueue();
    }

    void LambertComputeKernel::submitQueue() {
        if (bInJob)
            throw std::runtime_error("LambertComputeKernel: Pending Jobs!");

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
            glBindTexture(GL_TEXTURE_CUBE_MAP, job.mInputImage->id());
            glBindImageTexture(0, job.mInputImage->id(), 0, false, 0, 
                GL_READ_WRITE, GL_RGBA8);

            GL_ASSERT;

            glDispatchCompute(num_groups_x, 1, 1);

            offset += outputFloatSizeFor(job.mInputImage, mGroupSize);
        }

        bInJob = true;
    }

    float* LambertComputeKernel::results(uint job_id) {
        uint blockSize = 3 * LAMBERT_SH_COEFFS;
        return &mResultBuffer[blockSize * job_id];
    }

    void LambertComputeKernel::barrier() {

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

    void LambertComputeKernel::beginQueue() {
        mJobs.clear();
        mResultBuffer.clear();
        bInJob = false;
    }
}