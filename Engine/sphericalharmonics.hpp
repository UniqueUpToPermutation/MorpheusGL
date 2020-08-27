#pragma once

#include "core.hpp"

#include <cmath>
#include <Eigen/Dense>

namespace Morpheus {
	class SphericalHarmonics {
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
	};

	class CubemapJacobian {
	public:
		static void generate(const Eigen::VectorXf& inPhi, const Eigen::VectorXf& inTheta, Eigen::VectorXf* out);
		static void generate(const Eigen::VectorXd& inPhi, const Eigen::VectorXd& inTheta, Eigen::VectorXd* out);
		static void generate(const Eigen::VectorXf& inX, const Eigen::VectorXf& inY, const Eigen::VectorXf& inZ, Eigen::VectorXf* out);
		static void generate(const Eigen::VectorXd& inX, const Eigen::VectorXd& inY, const Eigen::VectorXd& inZ, Eigen::VectorXd* out);
	};
}