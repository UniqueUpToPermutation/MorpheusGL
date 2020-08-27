#pragma once

#include "core.hpp"
#include "shader.hpp"
#include "samplefunction.hpp"

namespace Morpheus {
	class Texture;
	class Shader;

	// Implements the Lambert BRDF
	template <typename VectorType = glm::vec3>
	class LambertKernel {
	public:
		typedef decltype(VectorType::x) scalar_t;

	private:
		std::minstd_rand generator;
		std::uniform_real_distribution<scalar_t> distribution;

	public:
		constexpr static bool HAS_WEIGHTS = true;
		typedef scalar_t WeightType;

		LambertKernel() :
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
			KernelProc<LambertKernel, FuncType>::apply(*this, input, output, sampleCount);
		}
	};
	
	class LambertBRDFFilter {
	private:
		ref<Shader> mCompute;

	public:
		LambertBRDFFilter(ref<Shader> compute);

		void dispatch(Texture inputLightField, Texture outputLightField);
	};
}