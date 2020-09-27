#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>
#include <engine/samplefunction.hpp>
#include <engine/sphericalharmonics.hpp>

#define LAMBERT_SH_COEFFS 9
#define LAMBERT_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE 32

namespace Morpheus {
	template <typename T>
	T factorial(T n)
	{
		if (n > 1)
			return n * factorial(n - 1);
		else
			return 1;
	}

	template <typename T>
	T ipow(T n, uint32_t pow) {
		if (pow == 0)
			return 1;
		else if (pow == 1)
			return n;
		else {
			return n * ipow(n, pow - 1);
		}
	}

	class Texture;
	class Shader;
	class LambertSHKernelCPU {
	public:
		typedef double scalar_t;

	private:
		std::vector<scalar_t> mFilter;
		int mLargestL;

	public:
		LambertSHKernelCPU(int largestL = 2) {
			mLargestL = largestL;
			mFilter.reserve((largestL + 1) * (largestL + 1));
			for (int l = 0; l <= largestL; ++l) {

				scalar_t coeff = 0.0;
				if (l == 0) {
					coeff = (scalar_t)1.0;
				}
				else if (l == 1) {
					coeff = (scalar_t)(2.0 / 3.0);
				}
				else if (l % 2 == 0) {
					coeff = (scalar_t)(2.0 *
						(scalar_t)ipow(-1, l / 2 - 1) / (scalar_t)((l + 2) * (l - 1))
						* (scalar_t)factorial(l) / (scalar_t)(ipow(2, l) * ipow(factorial(l / 2), 2)));
				}

				for (int m = -l; m <= l; ++m) {
					mFilter.emplace_back(coeff);
				}
			}
		}

		template <typename scalar_t>
		void applySH(const scalar_t shIn[], scalar_t shOut[], const uint modeCount, const uint channelCount) {
			for (int i = 0; i < channelCount; ++i) {
				for (int j = 0; j < modeCount; ++j) {
					shOut[j * channelCount + i] = mFilter[j] * shIn[j * channelCount + i];
				}
			}
		}

		template <typename scalar_t>
		void applySH(const std::vector<scalar_t>& shIn, std::vector<scalar_t>* shOut, const uint channelCount) {
			shOut->resize(shIn.size());
			applySH<scalar_t>(&shIn[0], &(*shOut)[0], shIn.size(), channelCount);
		}

		template <typename MatrixType>
		void applySH(const MatrixType& shIn, MatrixType* shOut) {
			shOut->resize(shIn.rows(), shIn.cols());
			for (int j = 0; j < shIn.cols(); ++j) {
				for (int i = 0; i < shIn.rows(); ++i) {
					(*shOut)(i, j) = mFilter[i] * shIn(i, j);
				}
			}
		}

		template <typename MatrixType, typename FuncReturnType> 
		void applyMC(const FunctionSphere<FuncReturnType>& func, MatrixType* shOut,
			uint32_t sampleCount = DEFAULT_SH_SAMPLE_COUNT) {
			MatrixType shIn;
			SphericalHarmonics::generateCoeffsMonteCarlo(func, &shIn, sampleCount, mLargestL);
			applySH(shIn, shOut);
		}

		template <typename MatrixType, typename FuncReturnType>
		void apply(const FunctionSphere<FuncReturnType>& func, MatrixType* shOut) {
			MatrixType shIn;
			SphericalHarmonics::generateCoeffs(func, &shIn, mLargestL);
			applySH(shIn, shOut);
		}
	};

	struct LambertComputeJob {
		Texture* mInputImage;
	};

	// Implements a lambert convolution in the SH basis on the GPU. Returns a set of 9 SH coefficients for 3 color channels.
	class LambertComputeKernel : public INodeOwner {
	private:
		GLuint mGPUOutputBuffer;
		
		std::vector<LambertComputeJob> mJobs;
		std::vector<float> mResultBuffer;

		Shader* mGPUBackend;
		ShaderUniform<uint> mOffsetUniform;
		LambertSHKernelCPU mSHTransferFunction;

		bool bInJob;
		uint mGroupSize;

	public:

		LambertComputeKernel(uint groupSize = LAMBERT_COMPUTE_KERNEL_DEFAULT_GROUP_SIZE);
		~LambertComputeKernel();

		void init() override;

		// These functions will submit a batch of jobs to the GPU
		void beginQueue();
		uint addJob(const LambertComputeJob& job);
		void submitQueue();

		// This function submits a single job to the GPU
		void submit(const LambertComputeJob& job);

		// Use this before attempting to access results of jobs
		void barrier();

		float* results(uint job_id = 0);

		inline uint shCount() const {
			return LAMBERT_SH_COEFFS;
		}
	};
	SET_NODE_ENUM(LambertComputeKernel, LAMBERT_COMPUTE_KERNEL);
}