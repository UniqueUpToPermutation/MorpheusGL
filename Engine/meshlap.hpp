//
// Created by Phil on 6/7/20.
//

#ifndef OPERATORAUGMENTATION_MESHLAP_H
#define OPERATORAUGMENTATION_MESHLAP_H

#include <Eigen/Sparse>
#include <Eigen/Dense>

namespace Morpheus {
    class HalfEdgeGeometry;

    void enumWeakLaplacian(const HalfEdgeGeometry& geo, std::vector<Eigen::Triplet<double>>* output);
    void enumWeakLaplacianPositiveDefinite(const HalfEdgeGeometry& geo, std::vector<Eigen::Triplet<double>>* output);
    void enumLaplacian(const HalfEdgeGeometry& geo, const Eigen::VectorXd& mass, std::vector<Eigen::Triplet<double>>* output);
    void enumLaplacian(const HalfEdgeGeometry& geo, std::vector<Eigen::Triplet<double>>* output);
    void enumLaplacianPositiveDefinite(const HalfEdgeGeometry& geo, std::vector<Eigen::Triplet<double>>* output);
    void weakLaplacian(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void weakLaplacianPositiveDefinite(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void massVector(const HalfEdgeGeometry& geo, Eigen::VectorXd* output);
    void massMatrix(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void laplacian(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void makeInteriorSelector(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void laplacianInteriorMinor(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void laplacianInteriorMinor(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output, Eigen::SparseMatrix<double>* interiorSelector);
    void laplacianPositiveDefinite(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void laplacianInteriorMinorPositiveDefinite(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output);
    void laplacianInteriorMinorPositiveDefinite(const HalfEdgeGeometry& geo, Eigen::SparseMatrix<double>* output, Eigen::SparseMatrix<double>* interiorSelector);
}

#endif //OPERATORAUGMENTATION_MESHLAP_H
