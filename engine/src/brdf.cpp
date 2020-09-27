#include <engine/brdf.hpp>

static const char* computeKernelSource = R"(

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;

layout(binding = 0) writeonly uniform image2D outputTexture;

const float PI = 3.1415926535897932384626433832795;

float radicalInverse_VdC(uint bits) {
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

vec3 importanceSampleGGX(vec2 Xi, float Roughness, vec3 N) {
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

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float G_smith(float Roughness, float NoV, float NoL) {
	float r = Roughness;
	float k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(NoV, k) * gaSchlickG1(NoL, k);
}

vec2 integrateBRDF(float Roughness, float NoV) {
	const uint NumSamples = 1024;

	vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V;
	V.x = sqrt(1.0 - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;

	for (uint i = 0; i < NumSamples; i++)
	{
		vec2 Xi = hammersley(i, NumSamples);
		vec3 H = importanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(L.z, 0.0, 1.0);
		float NoH = clamp(H.z, 0.0, 1.0);
		float VoH = clamp(dot(V, H), 0.0, 1.0);
		if (NoL > 0)
		{
			float G = G_smith(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return vec2(A, B) / NumSamples;
}

void main() {
    uvec2 pixLocation = gl_GlobalInvocationID.xy;

	vec2 uv = (vec2(pixLocation) + vec2(0.5, 0.5)) / imageSize(outputTexture);

	vec2 brdfValue = integrateBRDF(uv.x, uv.y);
	
	imageStore(outputTexture, ivec2(pixLocation), vec4(brdfValue, 0.0, 0.0));
}

)";

namespace Morpheus {
	CookTorranceLUTComputeKernel::CookTorranceLUTComputeKernel(uint computeGroupSize) : 
		INodeOwner(NodeType::COOK_TORRANCE_LUT_COMPUTE_KERNEL),
		mGPUBackend(nullptr), 
		mGroupSize(computeGroupSize) {
	}

	CookTorranceLUTComputeKernel::~CookTorranceLUTComputeKernel() {
		safeUnload(mGPUBackend);
	}

	Texture* CookTorranceLUTComputeKernel::submit(uint roughnessPixels, uint NoVPixels) {
		if (!mGPUBackend)
			std::runtime_error("CookTorranceLUTComputeKernel has not been initialized yet!");

		auto result = getFactory<Texture>()->makeTexture2DUnmanaged(roughnessPixels, NoVPixels, GL_RG16, 1);

		uint groups_x = result->width() / mGroupSize;
		uint groups_y = result->height() / mGroupSize;

		glUseProgram(mGPUBackend->id());
		glBindImageTexture(0, result->id(), 0, false, 0, GL_WRITE_ONLY, GL_RG16);
		glDispatchCompute(groups_x, groups_y, 1);

		return result;
	}

	void CookTorranceLUTComputeKernel::barrier() {
		glTextureBarrier();
	}

	void CookTorranceLUTComputeKernel::init() {
		if (!mGPUBackend) {

			std::string source(computeKernelSource);

            std::stringstream ss;
            ss << "#version 450" << std::endl;
            ss << "#define GROUP_SIZE " << mGroupSize << std::endl;
            ss << source;

            auto program = compileComputeKernel(ss.str());
                
            if (program == 0) {
                std::cout << "CookTorranceLUTComputeKernel: Error occured while compiling compute shader..." << std::endl;
                throw std::runtime_error("CookTorranceLUTComputeKernel: Compute shader failed to compile!");
            }

            // Add compute shader to this object's children for resource management
            mGPUBackend = getFactory<Shader>()->makeFromGL(program);
			createContentNode(mGPUBackend);
            addChild(mGPUBackend);
		}	
	}
}