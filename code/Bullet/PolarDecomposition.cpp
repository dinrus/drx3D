#include <drx3D/Maths/Linear/PolarDecomposition.h>
#include <drx3D/Maths/Linear/MinMax.h>

namespace
{
Scalar abs_column_sum(const Matrix3x3& a, i32 i)
{
	return Fabs(a[0][i]) + Fabs(a[1][i]) + Fabs(a[2][i]);
}

Scalar abs_row_sum(const Matrix3x3& a, i32 i)
{
	return Fabs(a[i][0]) + Fabs(a[i][1]) + Fabs(a[i][2]);
}

Scalar p1_norm(const Matrix3x3& a)
{
	const Scalar sum0 = abs_column_sum(a, 0);
	const Scalar sum1 = abs_column_sum(a, 1);
	const Scalar sum2 = abs_column_sum(a, 2);
	return d3Max(d3Max(sum0, sum1), sum2);
}

Scalar pinf_norm(const Matrix3x3& a)
{
	const Scalar sum0 = abs_row_sum(a, 0);
	const Scalar sum1 = abs_row_sum(a, 1);
	const Scalar sum2 = abs_row_sum(a, 2);
	return d3Max(d3Max(sum0, sum1), sum2);
}
}  // namespace

PolarDecomposition::PolarDecomposition(Scalar tolerance, u32 maxIterations)
	: m_tolerance(tolerance), m_maxIterations(maxIterations)
{
}

u32 PolarDecomposition::decompose(const Matrix3x3& a, Matrix3x3& u, Matrix3x3& h) const
{
	// Use the 'u' and 'h' matrices for intermediate calculations
	u = a;
	h = a.inverse();

	for (u32 i = 0; i < m_maxIterations; ++i)
	{
		const Scalar h_1 = p1_norm(h);
		const Scalar h_inf = pinf_norm(h);
		const Scalar u_1 = p1_norm(u);
		const Scalar u_inf = pinf_norm(u);

		const Scalar h_norm = h_1 * h_inf;
		const Scalar u_norm = u_1 * u_inf;

		// The matrix is effectively singular so we cannot invert it
		if (FuzzyZero(h_norm) || FuzzyZero(u_norm))
			break;

		const Scalar gamma = Pow(h_norm / u_norm, 0.25f);
		const Scalar inv_gamma = Scalar(1.0) / gamma;

		// Determine the delta to 'u'
		const Matrix3x3 delta = (u * (gamma - Scalar(2.0)) + h.transpose() * inv_gamma) * Scalar(0.5);

		// Update the matrices
		u += delta;
		h = u.inverse();

		// Check for convergence
		if (p1_norm(delta) <= m_tolerance * u_1)
		{
			h = u.transpose() * a;
			h = (h + h.transpose()) * 0.5;
			return i;
		}
	}

	// The algorithm has failed to converge to the specified tolerance, but we
	// want to make sure that the matrices returned are in the right form.
	h = u.transpose() * a;
	h = (h + h.transpose()) * 0.5;

	return m_maxIterations;
}

u32 PolarDecomposition::maxIterations() const
{
	return m_maxIterations;
}

u32 polarDecompose(const Matrix3x3& a, Matrix3x3& u, Matrix3x3& h)
{
	static PolarDecomposition polar;
	return polar.decompose(a, u, h);
}
