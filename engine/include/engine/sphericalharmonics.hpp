#pragma once

#include <engine/core.hpp>
#include <engine/samplefunction.hpp>

#include <cmath>
#include <Eigen/Dense>

#define DEFAULT_SH_SAMPLE_COUNT 50000u

#define SH_C_0 0.28209479177
#define SH_C_1 0.4886025119
#define SH_C_2n2 1.09254843059
#define SH_C_2n1 SH_C_2n2
#define SH_C_20 0.31539156525
#define SH_C_21 SH_C_2n2
#define SH_C_22 0.54627421529
#define SH_C_3n3 0.59004358992
#define SH_C_3n2 2.89061144264
#define SH_C_3n1 0.45704579946
#define SH_C_30 0.37317633259
#define SH_C_31 SH_C_3n1
#define SH_C_32 1.44530572132
#define SH_C_33 SH_C_3n3

namespace Morpheus {
	template <typename DestMatrixType, typename SourceType>
	struct WriteToEigen {
		static void copy(DestMatrixType* dest, uint32_t dest_row, const SourceType* src) {
			constexpr size_t len = SourceType::length();
			for (size_t i = 0; i < len; ++i) {
				(*dest)(dest_row, i) = (*src)[i];
			}
		}
	};

	template <typename DestMatrixType>
	struct WriteToEigen<DestMatrixType, float> {
		static void copy(DestMatrixType* dest, uint32_t dest_row, const float* src) {
			(*dest)(dest_row, 0) = (*src);
		}
	};

	template <typename DestMatrixType>
	struct WriteToEigen<DestMatrixType, double> {
		static void copy(DestMatrixType* dest, uint32_t dest_row, const double* src) {
			(*dest)(dest_row, 0) = (*src);
		}
	};

	template <typename scalar_t, int l, int m>
	struct sh {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			throw std::runtime_error("Spherical harmonics not implemented for desired (l, m)!");
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 0, 0> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)SH_C_0;
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 1, -1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_1 * y);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 1, 0> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_1 * z);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 1, 1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_1 * x);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 2, -2> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_2n2 * x * y);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 2, -1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_2n1 * y * z);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 2, 0> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_20 * (-x * x - y * y + 2.0 * z * z));
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 2, 1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_21 * z * x);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 2, 2> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_22 * (x * x - y * y));
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, -3> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_3n3 * (3 * x * x - y * y) * y);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, -2> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_3n2 * x * y * z);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, -1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_3n1 * y * (4.0 * z * z - x * x - y * y));
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, 0> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_30 * z * (2.0 * z * z - 3.0 * x * x - 3.0 * y * y));
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, 1> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_31 * x * (4.0 * z * z - x * x - y * y));
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, 2> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_32 * (x * x - y * y) * z);
		}
	};

	template <typename scalar_t>
	struct sh<scalar_t, 3, 3> {
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z) {
			return (scalar_t)(SH_C_33 * (x * x - 3.0 * y * y) * x);
		}
	};

	template <typename scalar_t_, int l, int m>
	struct sh_cubemap_dual {
		typedef scalar_t_ scalar_t;
		inline static scalar_t at(scalar_t x, scalar_t y, scalar_t z, scalar_t h) {
			scalar_t x_abs = std::abs(x);
			scalar_t y_abs = std::abs(y);
			scalar_t z_abs = std::abs(z);
			scalar_t u;
			scalar_t v;

			if (x_abs >= y_abs && x_abs >= z_abs) {
				u = y / x_abs;
				v = z / x_abs;
			}
			else if (y_abs >= x_abs && y_abs >= z_abs) {
				u = x / y_abs;
				v = z / y_abs;
			}
			else {
				u = x / z_abs;
				v = y / z_abs;
			}

			scalar_t mag2 = u * u + v * v + 1.0;
			scalar_t mag = std::sqrt(mag2);
			scalar_t jacobian = (scalar_t)1.0 / (mag2 * mag);
			scalar_t metricWeight = jacobian * h * h;
			return sh<scalar_t, l, m>::at(x, y, z) * metricWeight;
		}
	};

	class SphericalHarmonics {
	public:
		static void generateSphereSamples(
			const uint32_t count,
			Eigen::VectorXf* outX,
			Eigen::VectorXf* outY,
			Eigen::VectorXf* outZ);
		static void generateSphereSamples(
			const uint32_t count,
			Eigen::VectorXd* outX,
			Eigen::VectorXd* outY,
			Eigen::VectorXd* outZ);

		template <typename FuncReturnType, typename VectorType>
		static void generateCoeffMonteCarlo(FunctionSphere<FuncReturnType>& func, FuncReturnType* coeffsOut,
			const int l, const int m, uint32_t sampleCount = DEFAULT_SH_SAMPLE_COUNT) {
			VectorType X;
			VectorType Y;
			VectorType Z;
			generateSphereSamples(sampleCount, &X, &Y, &Z);
			VectorType shVal;
			generateFast(l, m, X, Y, Z, &shVal);
			FuncReturnType sum = glm::zero<FuncReturnType>();
			for (uint32_t i = 0; i < sampleCount; ++i) {
				glm::vec3 samplePos(X(i), Y(i), Z(i));
				auto sample = func(samplePos);
				sum += sample * shVal(i);
			}

			sum /= ((double)sampleCount);
			sum *= 4.0 * glm::pi<double>();
			*coeffsOut = sum;
		}

		template <typename FuncReturnType, typename VectorType>
		static void generateCoeff(FunctionSphere<FuncReturnType>& func, FuncReturnType* coeffsOut,
			const int l, const int m) {
			FuncReturnType sum = glm::zero<FuncReturnType>();
			
			auto& storage = func.storage();
			size_t size = storage.sampleCount();
			typedef typename get_scalar_t<VectorType>::scalar_t scalar_t;

			scalar_t h = (scalar_t)(2.0 / func.width());

			switch (l) {
			case 0: {
				switch (m) {
				case 0:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 0, 0>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				default:
					throw std::invalid_argument("Invalid (l, m)!");
					break;
				}
				}
				break;
			case 1: {
				switch (m) {
				case -1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 1, -1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 0:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 1, 0>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 1, 1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				default:
					throw std::invalid_argument("Invalid (l, m)!");
					break;
				}
				}
				break;
			case 2: {
				switch (m) {
				case -2:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 2, -2>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case -1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 2, -1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 0:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 2, 0>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 2, 1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 2:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 2, 2>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				default:
					throw std::invalid_argument("Invalid (l, m)!");
					break;
				}
				}
				break;
			case 3: {
				switch (m) {
				case -3:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, -3>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case -2:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, -2>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case -1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, -1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 0:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, 0>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 1:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, 1>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 2:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, 2>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				case 3:
					for (size_t i = 0; i < size; ++i) {
						auto loc = storage.getSampleLocation(i);
						sum += storage[i] * sh_cubemap_dual<scalar_t, 3, 3>::at((scalar_t)loc.x, (scalar_t)loc.y, (scalar_t)loc.z, h);
					}
					break;
				default:
					throw std::invalid_argument("Invalid (l, m)!");
					break;
				}
				}
				break;
			default:
				throw std::invalid_argument("Invalid (l, m)!");
				break;
			}
			
			*coeffsOut = sum;
		}

	private:
		template <typename FuncReturnType, typename MatrixType, typename VectorType>
		static void generateCoeffsMonteCarlo_(FunctionSphere<FuncReturnType>& func,
			MatrixType* coeffsOut, uint32_t sampleCount = DEFAULT_SH_SAMPLE_COUNT, const int largestL = 2) {

			int rows = (largestL + 1) * (largestL + 1);
			coeffsOut->resize(rows, getElementLength<FuncReturnType>());

			for (int row = 0, l = 0; l <= largestL; ++l) {
				for (int m = -l; m <= l; ++m, ++row) {
					FuncReturnType result;
					generateCoeffMonteCarlo<FuncReturnType, VectorType>(func, &result, l, m, sampleCount);
					WriteToEigen<MatrixType, FuncReturnType>::copy(coeffsOut, row, &result);
				}
			}
		}

		template <typename FuncReturnType, typename MatrixType, typename VectorType>
		static void generateCoeffs_(FunctionSphere<FuncReturnType>& func,
			MatrixType* coeffsOut, const int largestL = 2) {
			int rows = (largestL + 1) * (largestL + 1);
			coeffsOut->resize(rows, getElementLength<FuncReturnType>());

			for (int row = 0, l = 0; l <= largestL; ++l) {
				for (int m = -l; m <= l; ++m, ++row) {
					FuncReturnType result;
					generateCoeff<FuncReturnType, VectorType>(func, &result, l, m);
					WriteToEigen<MatrixType, FuncReturnType>::copy(coeffsOut, row, &result);
				}
			}
		}

	public:
		static void generateIrradianceModes(const Eigen::VectorXf& inX, 
			const Eigen::VectorXf& inY, 
			const Eigen::VectorXf& inZ, 
			Eigen::MatrixXf* out,
			const int highestL = 2);
		static void generateIrradianceModes(const Eigen::VectorXd& inX,
			const Eigen::VectorXd& inY,
			const Eigen::VectorXd& inZ, 
			Eigen::MatrixXd* out,
			const int highestL = 2);
		static void generateFast(const int l, const int m,
			const Eigen::VectorXf& inX,
			const Eigen::VectorXf& inY,
			const Eigen::VectorXf& inZ,
			Eigen::VectorXf* out);
		static void generateFast(const int l, const int m,
			const Eigen::VectorXd& inX,
			const Eigen::VectorXd& inY,
			const Eigen::VectorXd& inZ,
			Eigen::VectorXd* out);

		template <typename FuncReturnType>
		static inline void generateCoeffsMonteCarlo(FunctionSphere<FuncReturnType>& func,
			Eigen::MatrixXf* coeffsOut,
			uint32_t sampleCount = DEFAULT_SH_SAMPLE_COUNT,
			const int largestL = 2) {
			generateCoeffsMonteCarlo_<FuncReturnType, Eigen::MatrixXf, Eigen::VectorXf>(func, coeffsOut, sampleCount, largestL);
		}

		template <typename FuncReturnType>
		static inline void generateCoeffsMonteCarlo(FunctionSphere<FuncReturnType>& func,
			Eigen::MatrixXd* coeffsOut,
			uint32_t sampleCount = DEFAULT_SH_SAMPLE_COUNT,
			const int largestL = 2) {
			generateCoeffsMonteCarlo_<FuncReturnType, Eigen::MatrixXd, Eigen::VectorXd>(func, coeffsOut, sampleCount, largestL);
		}

		template <typename FuncReturnType>
		static inline void generateCoeffs(FunctionSphere<FuncReturnType>& func,
			Eigen::MatrixXd* coeffsOut,
			const int largestL = 2) {
			generateCoeffs_<FuncReturnType, Eigen::MatrixXd, Eigen::VectorXd>(func, coeffsOut, largestL);
		}

		template <typename FuncReturnType>
		static inline void generateCoeffs(FunctionSphere<FuncReturnType>& func,
			Eigen::MatrixXf* coeffsOut,
			const int largestL = 2) {
			generateCoeffs_<FuncReturnType, Eigen::MatrixXf, Eigen::VectorXf>(func, coeffsOut, largestL);
		}
	};

	class CubemapMetric {
	public:
		static void dual(const Eigen::VectorXf& primal, 
			Eigen::VectorXf* out);
		static void dual(const Eigen::VectorXd& primal, 
			Eigen::VectorXd* out);
		static void dual(const Eigen::MatrixXf& primal,
			Eigen::MatrixXf* out);
		static void dual(const Eigen::MatrixXd& primal,
			Eigen::MatrixXd* out);
	};
}