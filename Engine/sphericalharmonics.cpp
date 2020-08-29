#include "sphericalharmonics.hpp"

#include <glm/glm.hpp>

#define C_0 0.28209479177
#define C_1 0.4886025119
#define C_2n2 1.09254843059
#define C_2n1 C_2n2
#define C_20 0.31539156525
#define C_21 C_2n2
#define C_22 0.54627421529
#define C_3n3 0.59004358992
#define C_3n2 2.89061144264
#define C_3n1 0.45704579946
#define C_30 0.37317633259
#define C_31 C_3n1
#define C_32 1.44530572132
#define C_33 C_3n3

namespace Morpheus {
	template <typename scalar_t>
	using func3 = scalar_t(*)(scalar_t, scalar_t, scalar_t);

	template <typename VectorType, typename OutVectorType, typename scalar_t, func3<scalar_t> f>
	void generateInternal(const VectorType& inX, const VectorType& inY, const VectorType& inZ, OutVectorType* out) {
		assert(inX.size() == inY.size());
		assert(inY.size() == inZ.size());
		out->resize(inX.size());
		size_t size = inX.size();
		Eigen::Vector3d vec;
		for (size_t i = 0; i < size; ++i) {
			(*out)(i) = f(inX(i), inY(i), inZ(i));
		}
	}

	template <typename VectorType, typename OutVectorType, typename scalar_t>
	void generateInternalMode(const int l, const int m, 
		const VectorType& inX, const VectorType& inY, const VectorType& inZ, OutVectorType* out) {
		switch (l) {
		case 0: {
			struct stct {
				static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
					return (scalar_t)C_0;
				}
			};
			generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
			return;
		}	
		case 1:
			switch (m) {
			case -1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_1 * y);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_1 * z);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_1 * x);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			}
		case 2:
			switch (m) {
			case -2: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_2n2 * x * y);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case -1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_2n1 * y * z);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_20 * (-x * x - y * y + 2.0 * z * z));
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_21 * z * x);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 2: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_22 * (x * x - y * y));
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			}
		case 3:
			switch (m) {
			case -3: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_3n3 * (3 * x * x - y * y) * y);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case -2: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_3n2 * x * y * z);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case -1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_3n1 * y * (4.0 * z * z - x * x - y * y));
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_30 * z * (2.0 * z * z - 3.0 * x * x - 3.0 * y * y));
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_31 * x * (4.0 * z * z - x * x - y * y));
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 2: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_32 * (x * x - y * y) * z);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			case 3: {
				struct stct {
					static scalar_t f(scalar_t x, scalar_t y, scalar_t z) {
						return (scalar_t)(C_33 * (x * x - 3.0 * y * y) * x);
					}
				};
				generateInternal<VectorType, OutVectorType, scalar_t, &stct::f>(inX, inY, inZ, out);
				return;
			}
			}
		default:
			throw std::invalid_argument("l > 3 modes not supported!");
			return;
		}
	}

	template <typename VectorType, typename MatrixType, typename scalar_t>
	void generateIrradianceModesInternal(const VectorType& inX,
		const VectorType& inY,
		const VectorType& inZ,
		MatrixType* out,
		const int highestL) {
		int cols = 0;
		for (int l = 0; l <= highestL; ++l) 
			cols += 2 * l + 1;
		
		assert(inX.size() == inY.size());
		assert(inY.size() == inZ.size());

		out->resize(inX.size(), cols);

		VectorType vec;
		for (int l = 0, i = 0; l <= highestL; ++l) {
			for (int m = -l; m <= l; ++m, ++i) {
				generateInternalMode<VectorType, VectorType, scalar_t>(l, m, inX, inY, inZ, &vec);
				out->col(i) = vec;
			}
		}
	}

	void SphericalHarmonics::generateIrradianceModes(const Eigen::VectorXf& inX,
		const Eigen::VectorXf& inY,
		const Eigen::VectorXf& inZ,
		Eigen::MatrixXf* out,
		const int highestL) {

		generateIrradianceModesInternal<Eigen::VectorXf, Eigen::MatrixXf, float>(inX, inY, inZ, out, highestL);
	}

	void SphericalHarmonics::generateIrradianceModes(const Eigen::VectorXd& inX,
		const Eigen::VectorXd& inY,
		const Eigen::VectorXd& inZ,
		Eigen::MatrixXd* out,
		const int highestL) {
		generateIrradianceModesInternal<Eigen::VectorXd, Eigen::MatrixXd, double>(inX, inY, inZ, out, highestL);
	}

	void SphericalHarmonics::generateFast(const int l, const int m,
		const Eigen::VectorXf& inX,
		const Eigen::VectorXf& inY,
		const Eigen::VectorXf& inZ,
		Eigen::VectorXf* out) {
		generateInternalMode<Eigen::VectorXf, Eigen::VectorXf, float>(l, m, inX, inY, inZ, out);
	}

	void SphericalHarmonics::generateFast(const int l, const int m,
		const Eigen::VectorXd& inX,
		const Eigen::VectorXd& inY,
		const Eigen::VectorXd& inZ,
		Eigen::VectorXd* out) {
		generateInternalMode<Eigen::VectorXd, Eigen::VectorXd, double>(l, m, inX, inY, inZ, out);
	}

	template <typename VectorType, typename scalar_t>
	void cbmDual(const VectorType& primal,
		VectorType* out) {
		size_t n = (size_t)std::sqrt(primal.size() / 6);
		scalar_t h = (scalar_t)(2.0 / (scalar_t)n);
		scalar_t h2 = (scalar_t)(h * h);

		out->resize(primal.size());

		for (size_t i_face = 0, i = 0; i_face < 6; ++i_face) {
			for (size_t i_x = 0; i_x < n; ++i_x) {
				for (size_t i_y = 0; i_y < n; ++i_y) {
					scalar_t x = (scalar_t)((i_x + 0.5) * h - 1.0);
					scalar_t y = (scalar_t)((i_y + 0.5) * h - 1.0);
					scalar_t mag2 = x * x + y * y + 1;
					scalar_t mag = std::sqrt(mag2);
					scalar_t jacobian = (scalar_t)1.0 / (mag2 * mag);
					scalar_t metricWeight = jacobian * h2;
					(*out)(i++) = primal(i) * metricWeight;
				}
			}
		}
	}

	void CubemapMetric::dual(const Eigen::VectorXf& primal, 
		Eigen::VectorXf* out) {
		cbmDual<Eigen::VectorXf, float>(primal, out);
	}

	void CubemapMetric::dual(const Eigen::VectorXd& primal, 
		Eigen::VectorXd* out) {
		cbmDual<Eigen::VectorXd, double>(primal, out);
	}

	template <typename MatrixType, typename VectorType, typename scalar_t>
	void cbmDualMatrix(const MatrixType& primal,
		MatrixType* out) {
		out->resize(primal.rows(), primal.cols());

		size_t n = (size_t)std::sqrt(primal.rows() / 6);
		scalar_t h = (scalar_t)(2.0 / (scalar_t)n);
		scalar_t h2 = (scalar_t)(h * h);

		for (size_t i_col = 0; i_col < primal.cols(); ++i_col) {
			for (size_t i_face = 0, i = 0; i_face < 6; ++i_face) {
				for (size_t i_x = 0; i_x < n; ++i_x) {
					for (size_t i_y = 0; i_y < n; ++i_y) {
						scalar_t x = (scalar_t)((i_x + 0.5) * h - 1.0);
						scalar_t y = (scalar_t)((i_y + 0.5) * h - 1.0);
						scalar_t mag2 = x * x + y * y + 1;
						scalar_t mag = std::sqrt(mag2);
						scalar_t jacobian = (scalar_t)1.0 / (mag2 * mag);
						scalar_t metricWeight = jacobian * h2;
						(*out)(i++, i_col) = primal(i, i_col) * metricWeight;
					}
				}
			}
		}
	}

	void CubemapMetric::dual(const Eigen::MatrixXf& primal,
		Eigen::MatrixXf* out) {
		cbmDualMatrix<Eigen::MatrixXf, Eigen::VectorXf, float>(primal, out);
	}

	void CubemapMetric::dual(const Eigen::MatrixXd& primal,
		Eigen::MatrixXd* out) {
		cbmDualMatrix<Eigen::MatrixXd, Eigen::VectorXd, double>(primal,  out);
	}
}