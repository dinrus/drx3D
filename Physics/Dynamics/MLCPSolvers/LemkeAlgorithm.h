#ifndef DRX3D_NUMERICS_LEMKE_ALGORITHM_H_
#define DRX3D_NUMERICS_LEMKE_ALGORITHM_H_

#include <drx3D/Maths/Linear/MatrixX.h>

#include <vector>  //todo: replace by AlignedObjectArray

class LemkeAlgorithm
{
public:
	LemkeAlgorithm(const MatrixXu& M_, const VectorXu& q_, i32k& DEBUGLEVEL_ = 0) : DEBUGLEVEL(DEBUGLEVEL_)
	{
		setSystem(M_, q_);
	}

	/* GETTER / SETTER */
	/**
   * \brief return info of solution process
   */
	i32 getInfo()
	{
		return info;
	}

	/**
   * \brief get the number of steps until the solution was found
   */
	i32 getSteps(void)
	{
		return steps;
	}

	/**
   * \brief set system with Matrix M and vector q
   */
	void setSystem(const MatrixXu& M_, const VectorXu& q_)
	{
		m_M = M_;
		m_q = q_;
	}
	/***************************************************/

	/**
   * \brief solve algorithm adapted from : Fast Implementation of Lemke’s Algorithm for Rigid Body Contact Simulation (John E. Lloyd)
   */
	VectorXu solve(u32 maxloops = 0);

	virtual ~LemkeAlgorithm()
	{
	}

protected:
	i32 findLexicographicMinimum(const MatrixXu& A, i32k& pivotColIndex, i32k& z0Row, bool& isRayTermination);
	void GaussJordanEliminationStep(MatrixXu& A, i32 pivotRowIndex, i32 pivotColumnIndex, const AlignedObjectArray<i32>& basis);
	bool greaterZero(const VectorXu& vector);
	bool validBasis(const AlignedObjectArray<i32>& basis);

	MatrixXu m_M;
	VectorXu m_q;

	/**
   * \brief number of steps until the Lemke algorithm found a solution
   */
	u32 steps;

	/**
   * \brief define level of debug output
   */
	i32 DEBUGLEVEL;

	/**
   * \brief did the algorithm find a solution
   *
   * -1 : not successful
   *  0 : successful
   */
	i32 info;
};

#endif /* DRX3D_NUMERICS_LEMKE_ALGORITHM_H_ */
