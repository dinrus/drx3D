#ifndef POLARDECOMPOSITION_H
#define POLARDECOMPOSITION_H

#include <drx3D/Maths/Linear/Matrix3x3.h>

/**
 * This class is used to compute the polar decomposition of a matrix. In
 * general, the polar decomposition factorizes a matrix, A, into two parts: a
 * unitary matrix (U) and a positive, semi-definite Hermitian matrix (H).
 * However, in this particular implementation the original matrix, A, is
 * required to be a square 3x3 matrix with real elements. This means that U will
 * be an orthogonal matrix and H with be a positive-definite, symmetric matrix.
 */
class PolarDecomposition
{
public:
	/**
     * Creates an instance with optional parameters.
     *
     * @param tolerance     - the tolerance used to determine convergence of the
     *                        algorithm
     * @param maxIterations - the maximum number of iterations used to achieve
     *                        convergence
     */
	PolarDecomposition(Scalar tolerance = Scalar(0.0001),
						 u32 maxIterations = 16);

	/**
     * Decomposes a matrix into orthogonal and symmetric, positive-definite
     * parts. If the number of iterations returned by this function is equal to
     * the maximum number of iterations, the algorithm has failed to converge.
     *
     * @param a - the original matrix
     * @param u - the resulting orthogonal matrix
     * @param h - the resulting symmetric matrix
     *
     * @return the number of iterations performed by the algorithm.
     */
	u32 decompose(const Matrix3x3& a, Matrix3x3& u, Matrix3x3& h) const;

	/**
     * Returns the maximum number of iterations that this algorithm will perform
     * to achieve convergence.
     *
     * @return maximum number of iterations
     */
	u32 maxIterations() const;

private:
	Scalar m_tolerance;
	u32 m_maxIterations;
};

/**
 * This functions decomposes the matrix 'a' into two parts: an orthogonal matrix
 * 'u' and a symmetric, positive-definite matrix 'h'. If the number of
 * iterations returned by this function is equal to
 * PolarDecomposition::DEFAULT_MAX_ITERATIONS, the algorithm has failed to
 * converge.
 *
 * @param a - the original matrix
 * @param u - the resulting orthogonal matrix
 * @param h - the resulting symmetric matrix
 *
 * @return the number of iterations performed by the algorithm.
 */
u32 polarDecompose(const Matrix3x3& a, Matrix3x3& u, Matrix3x3& h);

#endif  // POLARDECOMPOSITION_H
