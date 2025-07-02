#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>

bool GjkEpaPenetrationDepthSolver::calcPenDepth(SimplexSolverInterface& simplexSolver,
												  const ConvexShape* pConvexA, const ConvexShape* pConvexB,
												  const Transform2& transformA, const Transform2& transformB,
												  Vec3& v, Vec3& wWitnessOnA, Vec3& wWitnessOnB,
												  class IDebugDraw* debugDraw)
{
	(void)debugDraw;
	(void)v;
	(void)simplexSolver;

	Vec3 guessVectors[] = {
		Vec3(transformB.getOrigin() - transformA.getOrigin()).safeNormalize(),
		Vec3(transformA.getOrigin() - transformB.getOrigin()).safeNormalize(),
		Vec3(0, 0, 1),
		Vec3(0, 1, 0),
		Vec3(1, 0, 0),
		Vec3(1, 1, 0),
		Vec3(1, 1, 1),
		Vec3(0, 1, 1),
		Vec3(1, 0, 1),
	};

	i32 numVectors = sizeof(guessVectors) / sizeof(Vec3);

	for (i32 i = 0; i < numVectors; i++)
	{
		simplexSolver.reset();
		Vec3 guessVector = guessVectors[i];

		GjkEpaSolver2::sResults results;

		if (GjkEpaSolver2::Penetration(pConvexA, transformA,
										 pConvexB, transformB,
										 guessVector, results))

		{
			wWitnessOnA = results.witnesses[0];
			wWitnessOnB = results.witnesses[1];
			v = results.normal;
			return true;
		}
		else
		{
			if (GjkEpaSolver2::Distance(pConvexA, transformA, pConvexB, transformB, guessVector, results))
			{
				wWitnessOnA = results.witnesses[0];
				wWitnessOnB = results.witnesses[1];
				v = results.normal;
				return false;
			}
		}
	}

	//failed to find a distance/penetration
	wWitnessOnA.setVal(0, 0, 0);
	wWitnessOnB.setVal(0, 0, 0);
	v.setVal(0, 0, 0);
	return false;
}
