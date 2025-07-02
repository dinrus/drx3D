#ifndef DRX3D_BATCHED_CONSTRAINTS_H
#define DRX3D_BATCHED_CONSTRAINTS_H

#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverConstraint.h>

class IDebugDraw;

struct BatchedConstraints
{
	enum BatchingMethod
	{
		BATCHING_METHOD_SPATIAL_GRID_2D,
		BATCHING_METHOD_SPATIAL_GRID_3D,
		BATCHING_METHOD_COUNT
	};
	struct Range
	{
		i32 begin;
		i32 end;

		Range() : begin(0), end(0) {}
		Range(i32 _beg, i32 _end) : begin(_beg), end(_end) {}
	};

	AlignedObjectArray<i32> m_constraintIndices;
	AlignedObjectArray<Range> m_batches;        // each batch is a range of indices in the m_constraintIndices array
	AlignedObjectArray<Range> m_phases;         // each phase is range of indices in the m_batches array
	AlignedObjectArray<char> m_phaseGrainSize;  // max grain size for each phase
	AlignedObjectArray<i32> m_phaseOrder;       // phases can be done in any order, so we can randomize the order here
	IDebugDraw* m_debugDrawer;

	static bool s_debugDrawBatches;

	BatchedConstraints() { m_debugDrawer = NULL; }
	void setup(ConstraintArray* constraints,
			   const AlignedObjectArray<SolverBody>& bodies,
			   BatchingMethod batchingMethod,
			   i32 minBatchSize,
			   i32 maxBatchSize,
			   AlignedObjectArray<char>* scratchMemory);
	bool validate(ConstraintArray* constraints, const AlignedObjectArray<SolverBody>& bodies) const;
};

#endif  // DRX3D_BATCHED_CONSTRAINTS_H
