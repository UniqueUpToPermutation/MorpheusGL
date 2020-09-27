#include <engine/ggx.hpp>

static const char* computeKernelSource = R"(

#define FACE_POSITIVE_X 0
#define FACE_NEGATIVE_X 1
#define FACE_POSITIVE_Y 2
#define FACE_NEGATIVE_Y 3
#define FACE_POSITIVE_Z 4
#define FACE_NEGATIVE_Z 5

#define TEXTURE_ARRAY_SIZE 8

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;

const float PI = 3.1415926535897932384626433832795;

// Uniforms
layout(binding = 0) uniform samplerCube inputTexture;
layout(binding = 0) writeonly uniform imageCube outputTextures[TEXTURE_ARRAY_SIZE];

uniform uint outputTextureCount;
uniform uint totalLevels;
uniform uint beginLevel;

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 hammersley(uint i, uint sampleCount)
{
	return vec2(float(i) / float(sampleCount), radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);

    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

vec3 prefilterEnvMap(float Roughness, vec3 R)
{
	const uint NumSamples = 1024;

	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3(0.0, 0.0, 0.0);
	float TotalWeight = 0.0;

	for (uint i = 0; i < NumSamples; i++)
	{
		vec2 Xi = hammersley(i, NumSamples);
		vec3 H = importanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(dot(N, L), 0.0, 1.0);

		if (NoL > 0)
		{
			PrefilteredColor += textureLod(inputTexture, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
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

vec2 getSurfacePosition(uvec2 texPosition, uint texSize) {
    vec2 uv = (vec2(texPosition) + 0.5) / float(texSize);
    return 2.0 * uv - 1.0;
}

void main() {
    uint face = gl_GlobalInvocationID.z;
    uvec2 pixLocation = gl_GlobalInvocationID.xy;

    uint size = imageSize(outputTextures[0]).x;
    int currentTextureIndex = 0;
	uint currentLevel = beginLevel;

    while (currentTextureIndex < outputTextureCount && 
		currentTextureIndex < TEXTURE_ARRAY_SIZE && 
		pixLocation.x < size && pixLocation.y < size) {
        vec2 surfacePos = getSurfacePosition(pixLocation, size);
        vec3 dir = getDirection(surfacePos, face);

        float roughness = float(currentLevel) / float(totalLevels - 1);

        // Do Monte Carlo
        vec3 specular = prefilterEnvMap(roughness, dir);

        imageStore(outputTextures[currentTextureIndex], ivec3(gl_GlobalInvocationID), vec4(specular, 1.0));

        size = size / 2;
        currentTextureIndex++;
		currentLevel++;
    }
}

)";

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
        
        if (job.mInputImage->type() != TextureType::CUBE_MAP)
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

			std::string source(computeKernelSource);

            std::stringstream ss;
            ss << "#version 450" << std::endl;
            ss << "#define GROUP_SIZE " << mGroupSize << std::endl;
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

	GGXComputeKernel::~GGXComputeKernel() {
		safeUnload(mCubemapSampler);
		safeUnload(mGPUBackend);
	}
}