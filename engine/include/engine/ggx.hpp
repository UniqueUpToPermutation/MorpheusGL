#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

#define GGX_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE 32

namespace Morpheus {
	struct GGXComputeJob {
		Texture* mInputImage;
		GLenum mOutputFormat;

		inline GGXComputeJob(Texture* input = nullptr, GLenum outputFormat = 0) :
			mInputImage(input), mOutputFormat(outputFormat) {
		}
	};

	class GGXComputeKernel : public INodeOwner {
	private:
		std::vector<GGXComputeJob> mJobs;
		std::vector<Texture*> mJobTextures;

		Sampler* mCubemapSampler;

		ShaderUniform<GLfloat> mRoughness;
		ShaderUniform<Texture> mOutputTextureImage;
		ShaderUniform<Sampler> mInputSamplerUniform;

		Shader* mGPUBackend;

		bool bInJob;
		uint mGroupSize;

	public:

		GGXComputeKernel(uint groupSize = GGX_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE);

		void beginQueue();
		Texture* addJobUnmanaged(const GGXComputeJob& job);
		void submitQueue();

		Texture* submitUnmanaged(const GGXComputeJob& job);

		inline Texture* addJob(const GGXComputeJob& job, INodeOwner* parent) {
			Texture* tex = addJobUnmanaged(job);
			createContentNode(tex, parent);
			return tex;
		}

		inline Texture* submit(const GGXComputeJob& job, INodeOwner* parent) {
			Texture* tex = submitUnmanaged(job);
			createContentNode(tex, parent);
			return tex;
		}

		void barrier();

		void init() override;
	};
	SET_NODE_ENUM(GGXComputeKernel, GGX_COMPUTE_KERNEL);
}