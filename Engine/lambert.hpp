#pragma once

#include "core.hpp"
#include "shader.hpp"
#include "samplefunction.hpp"
#include "sphericalharmonics.hpp"

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

	template <typename VectorType = glm::vec3>
	class LambertSphericalHarmonicsKernel {
	public:
		typedef typename get_scalar_t<VectorType>::scalar_t scalar_t;

	private:
		std::vector<scalar_t> mFilter;
		int mLargestL;

	public:
		LambertSphericalHarmonicsKernel(int largestL = 2) {
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

	// Implements the Lambert BRDF
	template <typename VectorType = glm::vec3>
	class LambertMonteCarloKernel {
	public:
		typedef typename get_scalar_t<VectorType>::scalar_t scalar_t;

	private:
		std::minstd_rand generator;
		std::uniform_real_distribution<scalar_t> distribution;

	public:
		constexpr static bool HAS_WEIGHTS = true;
		typedef scalar_t WeightType;

		LambertMonteCarloKernel() :
			generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count()),
			distribution(0.0, 1.0) {
		}

		VectorType sample(const VectorType& location, scalar_t* weight) {
			*weight = 10.0;

			// Sample from hemisphere and rotate
			scalar_t u = distribution(generator);
			scalar_t v = distribution(generator);

			scalar_t phi = 2 * glm::pi<scalar_t>() * u;
			scalar_t theta = std::acos(std::sqrt(v));

			scalar_t x = std::sin(phi) * std::sin(theta);
			scalar_t y = std::cos(phi) * std::sin(theta);
			scalar_t z = std::cos(theta);

			VectorType v_z = location;
			VectorType prod = glm::zero<VectorType>();
			if (std::abs(location.x) < std::abs(location.y)) {
				prod.x = (scalar_t)1.0;
			}
			else {
				prod.y = (scalar_t)1.0;
			}
			VectorType v_x = glm::cross(v_z, prod);
			VectorType v_y = glm::cross(v_x, v_z);
		
			return v_x * x + v_y * y + v_z * z;
		}

		template <typename FuncType>
		void apply(const FuncType& input, FuncType* output, uint32_t sampleCount = DEFAULT_KERNEL_SAMPLE_COUNT) {
			KernelProc<LambertMonteCarloKernel, FuncType>::apply(*this, input, output, sampleCount);
		}
	};
}