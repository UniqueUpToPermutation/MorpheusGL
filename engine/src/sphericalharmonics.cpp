#include <engine/sphericalharmonics.hpp>

#include <glm/glm.hpp>

namespace Morpheus {
	template <typename VectorType>
	void generateSphereSamplesInternal(const uint32_t count,
		VectorType* outX,
		VectorType* outY,
		VectorType* outZ) {

		outX->resize(count);
		outY->resize(count);
		outZ->resize(count);

		auto t = (*outX)(0);
		typedef decltype(t) scalar_t;
		std::minstd_rand generator;
		std::uniform_real_distribution<scalar_t> distribution(-1.0, 1.0);

		for (uint32_t i = 0; i < count; ++i) {
			scalar_t z = distribution(generator);
			scalar_t phi = glm::pi<scalar_t>() * distribution(generator);
			(*outZ)(i) = z;
			scalar_t sinTheta = (scalar_t)std::sqrt(1.0 - z * z);
			(*outX)(i) = std::cos(phi) * sinTheta;
			(*outY)(i) = std::sin(phi) * sinTheta;
		}
	}

	void SphericalHarmonics::generateSphereSamples(
		const uint32_t count,
		Eigen::VectorXf* outX,
		Eigen::VectorXf* outY,
		Eigen::VectorXf* outZ) {
		generateSphereSamplesInternal(count, outX, outY, outZ);
	}

	void SphericalHarmonics::generateSphereSamples(
		const uint32_t count,
		Eigen::VectorXd* outX,
		Eigen::VectorXd* outY,
		Eigen::VectorXd* outZ) {
		generateSphereSamplesInternal(count, outX, outY, outZ);
	}

	template <typename VectorType, typename OutVectorType, typename scalar_t, typename sh_struct>
	void generateInternal(const VectorType& inX, const VectorType& inY, const VectorType& inZ, OutVectorType* out) {
		assert(inX.size() == inY.size());
		assert(inY.size() == inZ.size());
		out->resize(inX.size());
		size_t size = inX.size();
		Eigen::Vector3d vec;
		for (size_t i = 0; i < size; ++i) {
			(*out)(i) = sh_struct::at(inX(i), inY(i), inZ(i));
		}
	}

	template <typename VectorType, typename OutVectorType, typename scalar_t>
	void generateInternalMode(const int l, const int m, 
		const VectorType& inX, const VectorType& inY, const VectorType& inZ, OutVectorType* out) {
		switch (l) {
		case 0: {
			generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 0, 0>>(inX, inY, inZ, out);
			return;
		}	
		case 1:
			switch (m) {
			case -1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 1, -1>>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 1, 0>>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 1, 1>>(inX, inY, inZ, out);
				return;
			}
			}
		case 2:
			switch (m) {
			case -2: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 2, -2>>(inX, inY, inZ, out);
				return;
			}
			case -1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 2, -1>>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 2, 0>>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 2, 1>>(inX, inY, inZ, out);
				return;
			}
			case 2: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 2, 2>>(inX, inY, inZ, out);
				return;
			}
			}
		case 3:
			switch (m) {
			case -3: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, -3>>(inX, inY, inZ, out);
				return;
			}
			case -2: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, -2>>(inX, inY, inZ, out);
				return;
			}
			case -1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, -1>>(inX, inY, inZ, out);
				return;
			}
			case 0: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, 0>>(inX, inY, inZ, out);
				return;
			}
			case 1: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, 1>>(inX, inY, inZ, out);
				return;
			}
			case 2: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, 2>>(inX, inY, inZ, out);
				return;
			}
			case 3: {
				generateInternal<VectorType, OutVectorType, scalar_t, sh<scalar_t, 3, 3>>(inX, inY, inZ, out);
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
		size_t cols = (size_t)primal.cols();

		for (size_t i_col = 0; i_col < cols; ++i_col) {
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