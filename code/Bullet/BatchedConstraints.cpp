#include <drx3D/Physics/Dynamics/ConstraintSolver/BatchedConstraints.h>

#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/StackAlloc.h>
#include <drx3D/Maths/Linear/Quickprof.h>

#include <string.h>  //for memset

#include <cmath>

i32k kNoMerge = -1;

bool BatchedConstraints::s_debugDrawBatches = false;

struct BatchedConstraintInfo
{
	i32 constraintIndex;
	i32 numConstraintRows;
	i32 bodyIds[2];
};

struct BatchInfo
{
	i32 numConstraints;
	i32 mergeIndex;

	BatchInfo() : numConstraints(0), mergeIndex(kNoMerge) {}
};

bool BatchedConstraints::validate(ConstraintArray* constraints, const AlignedObjectArray<SolverBody>& bodies) const
{
	//
	// validate: for debugging only. Verify coloring of bodies, that no body is touched by more than one batch in any given phase
	//
	i32 errors = 0;
	i32k kUnassignedBatch = -1;

	AlignedObjectArray<i32> bodyBatchId;
	for (i32 iPhase = 0; iPhase < m_phases.size(); ++iPhase)
	{
		bodyBatchId.resizeNoInitialize(0);
		bodyBatchId.resize(bodies.size(), kUnassignedBatch);
		const Range& phase = m_phases[iPhase];
		for (i32 iBatch = phase.begin; iBatch < phase.end; ++iBatch)
		{
			const Range& batch = m_batches[iBatch];
			for (i32 iiCons = batch.begin; iiCons < batch.end; ++iiCons)
			{
				i32 iCons = m_constraintIndices[iiCons];
				const SolverConstraint& cons = constraints->at(iCons);
				const SolverBody& bodyA = bodies[cons.m_solverBodyIdA];
				const SolverBody& bodyB = bodies[cons.m_solverBodyIdB];
				if (!bodyA.internalGetInvMass().isZero())
				{
					i32 thisBodyBatchId = bodyBatchId[cons.m_solverBodyIdA];
					if (thisBodyBatchId == kUnassignedBatch)
					{
						bodyBatchId[cons.m_solverBodyIdA] = iBatch;
					}
					else if (thisBodyBatchId != iBatch)
					{
						Assert(!"dynamic body is used in 2 different batches in the same phase");
						errors++;
					}
				}
				if (!bodyB.internalGetInvMass().isZero())
				{
					i32 thisBodyBatchId = bodyBatchId[cons.m_solverBodyIdB];
					if (thisBodyBatchId == kUnassignedBatch)
					{
						bodyBatchId[cons.m_solverBodyIdB] = iBatch;
					}
					else if (thisBodyBatchId != iBatch)
					{
						Assert(!"dynamic body is used in 2 different batches in the same phase");
						errors++;
					}
				}
			}
		}
	}
	return errors == 0;
}

static void debugDrawSingleBatch(const BatchedConstraints* bc,
								 ConstraintArray* constraints,
								 const AlignedObjectArray<SolverBody>& bodies,
								 i32 iBatch,
								 const Vec3& color,
								 const Vec3& offset)
{
	if (bc && bc->m_debugDrawer && iBatch < bc->m_batches.size())
	{
		const BatchedConstraints::Range& b = bc->m_batches[iBatch];
		for (i32 iiCon = b.begin; iiCon < b.end; ++iiCon)
		{
			i32 iCon = bc->m_constraintIndices[iiCon];
			const SolverConstraint& con = constraints->at(iCon);
			i32 iBody0 = con.m_solverBodyIdA;
			i32 iBody1 = con.m_solverBodyIdB;
			Vec3 pos0 = bodies[iBody0].getWorldTransform().getOrigin() + offset;
			Vec3 pos1 = bodies[iBody1].getWorldTransform().getOrigin() + offset;
			bc->m_debugDrawer->drawLine(pos0, pos1, color);
		}
	}
}

static void debugDrawPhase(const BatchedConstraints* bc,
						   ConstraintArray* constraints,
						   const AlignedObjectArray<SolverBody>& bodies,
						   i32 iPhase,
						   const Vec3& color0,
						   const Vec3& color1,
						   const Vec3& offset)
{
	DRX3D_PROFILE("debugDrawPhase");
	if (bc && bc->m_debugDrawer && iPhase < bc->m_phases.size())
	{
		const BatchedConstraints::Range& phase = bc->m_phases[iPhase];
		for (i32 iBatch = phase.begin; iBatch < phase.end; ++iBatch)
		{
			float tt = float(iBatch - phase.begin) / float(d3Max(1, phase.end - phase.begin - 1));
			Vec3 col = lerp(color0, color1, tt);
			debugDrawSingleBatch(bc, constraints, bodies, iBatch, col, offset);
		}
	}
}

static void debugDrawAllBatches(const BatchedConstraints* bc,
								ConstraintArray* constraints,
								const AlignedObjectArray<SolverBody>& bodies)
{
	DRX3D_PROFILE("debugDrawAllBatches");
	if (bc && bc->m_debugDrawer && bc->m_phases.size() > 0)
	{
		Vec3 bboxMin(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
		Vec3 bboxMax = -bboxMin;
		for (i32 iBody = 0; iBody < bodies.size(); ++iBody)
		{
			const Vec3& pos = bodies[iBody].getWorldTransform().getOrigin();
			bboxMin.setMin(pos);
			bboxMax.setMax(pos);
		}
		Vec3 bboxExtent = bboxMax - bboxMin;
		Vec3 offsetBase = Vec3(0, bboxExtent.y() * 1.1f, 0);
		Vec3 offsetStep = Vec3(0, 0, bboxExtent.z() * 1.1f);
		i32 numPhases = bc->m_phases.size();
		for (i32 iPhase = 0; iPhase < numPhases; ++iPhase)
		{
			float b = float(iPhase) / float(numPhases - 1);
			Vec3 color0 = Vec3(1, 0, b);
			Vec3 color1 = Vec3(0, 1, b);
			Vec3 offset = offsetBase + offsetStep * (float(iPhase) - float(numPhases - 1) * 0.5);
			debugDrawPhase(bc, constraints, bodies, iPhase, color0, color1, offset);
		}
	}
}

static void initBatchedBodyDynamicFlags(AlignedObjectArray<bool>* outBodyDynamicFlags, const AlignedObjectArray<SolverBody>& bodies)
{
	DRX3D_PROFILE("initBatchedBodyDynamicFlags");
	AlignedObjectArray<bool>& bodyDynamicFlags = *outBodyDynamicFlags;
	bodyDynamicFlags.resizeNoInitialize(bodies.size());
	for (i32 i = 0; i < bodies.size(); ++i)
	{
		const SolverBody& body = bodies[i];
		bodyDynamicFlags[i] = (body.internalGetInvMass().x() > Scalar(0));
	}
}

static i32 runLengthEncodeConstraintInfo(BatchedConstraintInfo* outConInfos, i32 numConstraints)
{
	DRX3D_PROFILE("runLengthEncodeConstraintInfo");
	// detect and run-length encode constraint rows that repeat the same bodies
	i32 iDest = 0;
	i32 iSrc = 0;
	while (iSrc < numConstraints)
	{
		const BatchedConstraintInfo& srcConInfo = outConInfos[iSrc];
		BatchedConstraintInfo& conInfo = outConInfos[iDest];
		conInfo.constraintIndex = iSrc;
		conInfo.bodyIds[0] = srcConInfo.bodyIds[0];
		conInfo.bodyIds[1] = srcConInfo.bodyIds[1];
		while (iSrc < numConstraints && outConInfos[iSrc].bodyIds[0] == srcConInfo.bodyIds[0] && outConInfos[iSrc].bodyIds[1] == srcConInfo.bodyIds[1])
		{
			++iSrc;
		}
		conInfo.numConstraintRows = iSrc - conInfo.constraintIndex;
		++iDest;
	}
	return iDest;
}

struct ReadSolverConstraintsLoop : public IParallelForBody
{
	BatchedConstraintInfo* m_outConInfos;
	ConstraintArray* m_constraints;

	ReadSolverConstraintsLoop(BatchedConstraintInfo* outConInfos, ConstraintArray* constraints)
	{
		m_outConInfos = outConInfos;
		m_constraints = constraints;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		for (i32 i = iBegin; i < iEnd; ++i)
		{
			BatchedConstraintInfo& conInfo = m_outConInfos[i];
			const SolverConstraint& con = m_constraints->at(i);
			conInfo.bodyIds[0] = con.m_solverBodyIdA;
			conInfo.bodyIds[1] = con.m_solverBodyIdB;
			conInfo.constraintIndex = i;
			conInfo.numConstraintRows = 1;
		}
	}
};

static i32 initBatchedConstraintInfo(BatchedConstraintInfo* outConInfos, ConstraintArray* constraints)
{
	DRX3D_PROFILE("initBatchedConstraintInfo");
	i32 numConstraints = constraints->size();
	bool inParallel = true;
	if (inParallel)
	{
		ReadSolverConstraintsLoop loop(outConInfos, constraints);
		i32 grainSize = 1200;
		ParallelFor(0, numConstraints, grainSize, loop);
	}
	else
	{
		for (i32 i = 0; i < numConstraints; ++i)
		{
			BatchedConstraintInfo& conInfo = outConInfos[i];
			const SolverConstraint& con = constraints->at(i);
			conInfo.bodyIds[0] = con.m_solverBodyIdA;
			conInfo.bodyIds[1] = con.m_solverBodyIdB;
			conInfo.constraintIndex = i;
			conInfo.numConstraintRows = 1;
		}
	}
	bool useRunLengthEncoding = true;
	if (useRunLengthEncoding)
	{
		numConstraints = runLengthEncodeConstraintInfo(outConInfos, numConstraints);
	}
	return numConstraints;
}

static void expandConstraintRowsInPlace(i32* constraintBatchIds, const BatchedConstraintInfo* conInfos, i32 numConstraints, i32 numConstraintRows)
{
	DRX3D_PROFILE("expandConstraintRowsInPlace");
	if (numConstraintRows > numConstraints)
	{
		// we walk the array in reverse to avoid overwriteing
		for (i32 iCon = numConstraints - 1; iCon >= 0; --iCon)
		{
			const BatchedConstraintInfo& conInfo = conInfos[iCon];
			i32 iBatch = constraintBatchIds[iCon];
			for (i32 i = conInfo.numConstraintRows - 1; i >= 0; --i)
			{
				i32 iDest = conInfo.constraintIndex + i;
				Assert(iDest >= iCon);
				Assert(iDest >= 0 && iDest < numConstraintRows);
				constraintBatchIds[iDest] = iBatch;
			}
		}
	}
}

static void expandConstraintRows(i32* destConstraintBatchIds, i32k* srcConstraintBatchIds, const BatchedConstraintInfo* conInfos, i32 numConstraints, i32 numConstraintRows)
{
	DRX3D_PROFILE("expandConstraintRows");
	for (i32 iCon = 0; iCon < numConstraints; ++iCon)
	{
		const BatchedConstraintInfo& conInfo = conInfos[iCon];
		i32 iBatch = srcConstraintBatchIds[iCon];
		for (i32 i = 0; i < conInfo.numConstraintRows; ++i)
		{
			i32 iDest = conInfo.constraintIndex + i;
			Assert(iDest >= iCon);
			Assert(iDest >= 0 && iDest < numConstraintRows);
			destConstraintBatchIds[iDest] = iBatch;
		}
	}
}

struct ExpandConstraintRowsLoop : public IParallelForBody
{
	i32* m_destConstraintBatchIds;
	i32k* m_srcConstraintBatchIds;
	const BatchedConstraintInfo* m_conInfos;
	i32 m_numConstraintRows;

	ExpandConstraintRowsLoop(i32* destConstraintBatchIds, i32k* srcConstraintBatchIds, const BatchedConstraintInfo* conInfos, i32 numConstraintRows)
	{
		m_destConstraintBatchIds = destConstraintBatchIds;
		m_srcConstraintBatchIds = srcConstraintBatchIds;
		m_conInfos = conInfos;
		m_numConstraintRows = numConstraintRows;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		expandConstraintRows(m_destConstraintBatchIds, m_srcConstraintBatchIds + iBegin, m_conInfos + iBegin, iEnd - iBegin, m_numConstraintRows);
	}
};

static void expandConstraintRowsMt(i32* destConstraintBatchIds, i32k* srcConstraintBatchIds, const BatchedConstraintInfo* conInfos, i32 numConstraints, i32 numConstraintRows)
{
	DRX3D_PROFILE("expandConstraintRowsMt");
	ExpandConstraintRowsLoop loop(destConstraintBatchIds, srcConstraintBatchIds, conInfos, numConstraintRows);
	i32 grainSize = 600;
	ParallelFor(0, numConstraints, grainSize, loop);
}

static void initBatchedConstraintInfoArray(AlignedObjectArray<BatchedConstraintInfo>* outConInfos, ConstraintArray* constraints)
{
	DRX3D_PROFILE("initBatchedConstraintInfoArray");
	AlignedObjectArray<BatchedConstraintInfo>& conInfos = *outConInfos;
	i32 numConstraints = constraints->size();
	conInfos.resizeNoInitialize(numConstraints);

	i32 newSize = initBatchedConstraintInfo(&outConInfos->at(0), constraints);
	conInfos.resizeNoInitialize(newSize);
}

static void mergeSmallBatches(BatchInfo* batches, i32 iBeginBatch, i32 iEndBatch, i32 minBatchSize, i32 maxBatchSize)
{
	DRX3D_PROFILE("mergeSmallBatches");
	for (i32 iBatch = iEndBatch - 1; iBatch >= iBeginBatch; --iBatch)
	{
		BatchInfo& batch = batches[iBatch];
		if (batch.mergeIndex == kNoMerge && batch.numConstraints > 0 && batch.numConstraints < minBatchSize)
		{
			for (i32 iDestBatch = iBatch - 1; iDestBatch >= iBeginBatch; --iDestBatch)
			{
				BatchInfo& destBatch = batches[iDestBatch];
				if (destBatch.mergeIndex == kNoMerge && (destBatch.numConstraints + batch.numConstraints) < maxBatchSize)
				{
					destBatch.numConstraints += batch.numConstraints;
					batch.numConstraints = 0;
					batch.mergeIndex = iDestBatch;
					break;
				}
			}
		}
	}
	// flatten mergeIndexes
	// e.g. in case where A was merged into B and then B was merged into C, we need A to point to C instead of B
	// Note: loop goes forward through batches because batches always merge from higher indexes to lower,
	//     so by going from low to high it reduces the amount of trail-following
	for (i32 iBatch = iBeginBatch; iBatch < iEndBatch; ++iBatch)
	{
		BatchInfo& batch = batches[iBatch];
		if (batch.mergeIndex != kNoMerge)
		{
			i32 iMergeDest = batches[batch.mergeIndex].mergeIndex;
			// follow trail of merges to the end
			while (iMergeDest != kNoMerge)
			{
				i32 iNext = batches[iMergeDest].mergeIndex;
				if (iNext == kNoMerge)
				{
					batch.mergeIndex = iMergeDest;
					break;
				}
				iMergeDest = iNext;
			}
		}
	}
}

static void updateConstraintBatchIdsForMerges(i32* constraintBatchIds, i32 numConstraints, const BatchInfo* batches, i32 numBatches)
{
	DRX3D_PROFILE("updateConstraintBatchIdsForMerges");
	// update batchIds to account for merges
	for (i32 i = 0; i < numConstraints; ++i)
	{
		i32 iBatch = constraintBatchIds[i];
		Assert(iBatch < numBatches);
		// if this constraint references a batch that was merged into another batch
		if (batches[iBatch].mergeIndex != kNoMerge)
		{
			// update batchId
			constraintBatchIds[i] = batches[iBatch].mergeIndex;
		}
	}
}

struct UpdateConstraintBatchIdsForMergesLoop : public IParallelForBody
{
	i32* m_constraintBatchIds;
	const BatchInfo* m_batches;
	i32 m_numBatches;

	UpdateConstraintBatchIdsForMergesLoop(i32* constraintBatchIds, const BatchInfo* batches, i32 numBatches)
	{
		m_constraintBatchIds = constraintBatchIds;
		m_batches = batches;
		m_numBatches = numBatches;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("UpdateConstraintBatchIdsForMergesLoop");
		updateConstraintBatchIdsForMerges(m_constraintBatchIds + iBegin, iEnd - iBegin, m_batches, m_numBatches);
	}
};

static void updateConstraintBatchIdsForMergesMt(i32* constraintBatchIds, i32 numConstraints, const BatchInfo* batches, i32 numBatches)
{
	DRX3D_PROFILE("updateConstraintBatchIdsForMergesMt");
	UpdateConstraintBatchIdsForMergesLoop loop(constraintBatchIds, batches, numBatches);
	i32 grainSize = 800;
	ParallelFor(0, numConstraints, grainSize, loop);
}

inline bool BatchCompare(const BatchedConstraints::Range& a, const BatchedConstraints::Range& b)
{
	i32 lenA = a.end - a.begin;
	i32 lenB = b.end - b.begin;
	return lenA > lenB;
}

static void writeOutConstraintIndicesForRangeOfBatches(BatchedConstraints* bc,
													   i32k* constraintBatchIds,
													   i32 numConstraints,
													   i32* constraintIdPerBatch,
													   i32 batchBegin,
													   i32 batchEnd)
{
	DRX3D_PROFILE("writeOutConstraintIndicesForRangeOfBatches");
	for (i32 iCon = 0; iCon < numConstraints; ++iCon)
	{
		i32 iBatch = constraintBatchIds[iCon];
		if (iBatch >= batchBegin && iBatch < batchEnd)
		{
			i32 iDestCon = constraintIdPerBatch[iBatch];
			constraintIdPerBatch[iBatch] = iDestCon + 1;
			bc->m_constraintIndices[iDestCon] = iCon;
		}
	}
}

struct WriteOutConstraintIndicesLoop : public IParallelForBody
{
	BatchedConstraints* m_batchedConstraints;
	i32k* m_constraintBatchIds;
	i32 m_numConstraints;
	i32* m_constraintIdPerBatch;
	i32 m_maxNumBatchesPerPhase;

	WriteOutConstraintIndicesLoop(BatchedConstraints* bc, i32k* constraintBatchIds, i32 numConstraints, i32* constraintIdPerBatch, i32 maxNumBatchesPerPhase)
	{
		m_batchedConstraints = bc;
		m_constraintBatchIds = constraintBatchIds;
		m_numConstraints = numConstraints;
		m_constraintIdPerBatch = constraintIdPerBatch;
		m_maxNumBatchesPerPhase = maxNumBatchesPerPhase;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("WriteOutConstraintIndicesLoop");
		i32 batchBegin = iBegin * m_maxNumBatchesPerPhase;
		i32 batchEnd = iEnd * m_maxNumBatchesPerPhase;
		writeOutConstraintIndicesForRangeOfBatches(m_batchedConstraints,
												   m_constraintBatchIds,
												   m_numConstraints,
												   m_constraintIdPerBatch,
												   batchBegin,
												   batchEnd);
	}
};

static void writeOutConstraintIndicesMt(BatchedConstraints* bc,
										i32k* constraintBatchIds,
										i32 numConstraints,
										i32* constraintIdPerBatch,
										i32 maxNumBatchesPerPhase,
										i32 numPhases)
{
	DRX3D_PROFILE("writeOutConstraintIndicesMt");
	bool inParallel = true;
	if (inParallel)
	{
		WriteOutConstraintIndicesLoop loop(bc, constraintBatchIds, numConstraints, constraintIdPerBatch, maxNumBatchesPerPhase);
		ParallelFor(0, numPhases, 1, loop);
	}
	else
	{
		for (i32 iCon = 0; iCon < numConstraints; ++iCon)
		{
			i32 iBatch = constraintBatchIds[iCon];
			i32 iDestCon = constraintIdPerBatch[iBatch];
			constraintIdPerBatch[iBatch] = iDestCon + 1;
			bc->m_constraintIndices[iDestCon] = iCon;
		}
	}
}

static void writeGrainSizes(BatchedConstraints* bc)
{
	typedef BatchedConstraints::Range Range;
	i32 numPhases = bc->m_phases.size();
	bc->m_phaseGrainSize.resizeNoInitialize(numPhases);
	i32 numThreads = GetTaskScheduler()->getNumThreads();
	for (i32 iPhase = 0; iPhase < numPhases; ++iPhase)
	{
		const Range& phase = bc->m_phases[iPhase];
		i32 numBatches = phase.end - phase.begin;
		float grainSize = std::floor((0.25f * numBatches / float(numThreads)) + 0.0f);
		bc->m_phaseGrainSize[iPhase] = d3Max(1, i32(grainSize));
	}
}

static void writeOutBatches(BatchedConstraints* bc,
							i32k* constraintBatchIds,
							i32 numConstraints,
							const BatchInfo* batches,
							i32* batchWork,
							i32 maxNumBatchesPerPhase,
							i32 numPhases)
{
	DRX3D_PROFILE("writeOutBatches");
	typedef BatchedConstraints::Range Range;
	bc->m_constraintIndices.reserve(numConstraints);
	bc->m_batches.resizeNoInitialize(0);
	bc->m_phases.resizeNoInitialize(0);

	//i32 maxNumBatches = numPhases * maxNumBatchesPerPhase;
	{
		i32* constraintIdPerBatch = batchWork;  // for each batch, keep an index into the next available slot in the m_constraintIndices array
		i32 iConstraint = 0;
		for (i32 iPhase = 0; iPhase < numPhases; ++iPhase)
		{
			i32 curPhaseBegin = bc->m_batches.size();
			i32 iBegin = iPhase * maxNumBatchesPerPhase;
			i32 iEnd = iBegin + maxNumBatchesPerPhase;
			for (i32 i = iBegin; i < iEnd; ++i)
			{
				const BatchInfo& batch = batches[i];
				i32 curBatchBegin = iConstraint;
				constraintIdPerBatch[i] = curBatchBegin;  // record the start of each batch in m_constraintIndices array
				i32 numConstraints = batch.numConstraints;
				iConstraint += numConstraints;
				if (numConstraints > 0)
				{
					bc->m_batches.push_back(Range(curBatchBegin, iConstraint));
				}
			}
			// if any batches were emitted this phase,
			if (bc->m_batches.size() > curPhaseBegin)
			{
				// output phase
				bc->m_phases.push_back(Range(curPhaseBegin, bc->m_batches.size()));
			}
		}

		Assert(iConstraint == numConstraints);
		bc->m_constraintIndices.resizeNoInitialize(numConstraints);
		writeOutConstraintIndicesMt(bc, constraintBatchIds, numConstraints, constraintIdPerBatch, maxNumBatchesPerPhase, numPhases);
	}
	// for each phase
	for (i32 iPhase = 0; iPhase < bc->m_phases.size(); ++iPhase)
	{
		// sort the batches from largest to smallest (can be helpful to some task schedulers)
		const Range& curBatches = bc->m_phases[iPhase];
		bc->m_batches.quickSortInternal(BatchCompare, curBatches.begin, curBatches.end - 1);
	}
	bc->m_phaseOrder.resize(bc->m_phases.size());
	for (i32 i = 0; i < bc->m_phases.size(); ++i)
	{
		bc->m_phaseOrder[i] = i;
	}
	writeGrainSizes(bc);
}

//
// PreallocatedMemoryHelper -- helper object for allocating a number of chunks of memory in a single contiguous block.
//                             It is generally more efficient to do a single larger allocation than many smaller allocations.
//
// Example Usage:
//
//  Vec3* bodyPositions = NULL;
//  BatchedConstraintInfo* conInfos = NULL;
//  {
//    PreallocatedMemoryHelper<8> memHelper;
//    memHelper.addChunk( (uk *) &bodyPositions, sizeof( Vec3 ) * bodies.size() );
//    memHelper.addChunk( (uk *) &conInfos, sizeof( BatchedConstraintInfo ) * numConstraints );
//    uk memPtr = malloc( memHelper.getSizeToAllocate() );  // allocate the memory
//    memHelper.setChunkPointers( memPtr );  // update pointers to chunks
//  }
template <i32 N>
class PreallocatedMemoryHelper
{
	struct Chunk
	{
		uk * ptr;
		size_t size;
	};
	Chunk m_chunks[N];
	i32 m_numChunks;

public:
	PreallocatedMemoryHelper() { m_numChunks = 0; }
	void addChunk(uk * ptr, size_t sz)
	{
		Assert(m_numChunks < N);
		if (m_numChunks < N)
		{
			Chunk& chunk = m_chunks[m_numChunks];
			chunk.ptr = ptr;
			chunk.size = sz;
			m_numChunks++;
		}
	}
	size_t getSizeToAllocate() const
	{
		size_t totalSize = 0;
		for (i32 i = 0; i < m_numChunks; ++i)
		{
			totalSize += m_chunks[i].size;
		}
		return totalSize;
	}
	void setChunkPointers(uk mem) const
	{
		size_t totalSize = 0;
		for (i32 i = 0; i < m_numChunks; ++i)
		{
			const Chunk& chunk = m_chunks[i];
			tuk chunkPtr = static_cast<tuk>(mem) + totalSize;
			*chunk.ptr = chunkPtr;
			totalSize += chunk.size;
		}
	}
};

static Vec3 findMaxDynamicConstraintExtent(
	Vec3* bodyPositions,
	bool* bodyDynamicFlags,
	BatchedConstraintInfo* conInfos,
	i32 numConstraints,
	i32 numBodies)
{
	DRX3D_PROFILE("findMaxDynamicConstraintExtent");
	Vec3 consExtent = Vec3(1, 1, 1) * 0.001;
	for (i32 iCon = 0; iCon < numConstraints; ++iCon)
	{
		const BatchedConstraintInfo& con = conInfos[iCon];
		i32 iBody0 = con.bodyIds[0];
		i32 iBody1 = con.bodyIds[1];
		Assert(iBody0 >= 0 && iBody0 < numBodies);
		Assert(iBody1 >= 0 && iBody1 < numBodies);
		// is it a dynamic constraint?
		if (bodyDynamicFlags[iBody0] && bodyDynamicFlags[iBody1])
		{
			Vec3 delta = bodyPositions[iBody1] - bodyPositions[iBody0];
			consExtent.setMax(delta.absolute());
		}
	}
	return consExtent;
}

struct IntVec3
{
	i32 m_ints[3];

	SIMD_FORCE_INLINE i32k& operator[](i32 i) const { return m_ints[i]; }
	SIMD_FORCE_INLINE i32& operator[](i32 i) { return m_ints[i]; }
};

struct AssignConstraintsToGridBatchesParams
{
	bool* bodyDynamicFlags;
	IntVec3* bodyGridCoords;
	i32 numBodies;
	BatchedConstraintInfo* conInfos;
	i32* constraintBatchIds;
	IntVec3 gridChunkDim;
	i32 maxNumBatchesPerPhase;
	i32 numPhases;
	i32 phaseMask;

	AssignConstraintsToGridBatchesParams()
	{
		memset(this, 0, sizeof(*this));
	}
};

static void assignConstraintsToGridBatches(const AssignConstraintsToGridBatchesParams& params, i32 iConBegin, i32 iConEnd)
{
	DRX3D_PROFILE("assignConstraintsToGridBatches");
	// (can be done in parallel)
	for (i32 iCon = iConBegin; iCon < iConEnd; ++iCon)
	{
		const BatchedConstraintInfo& con = params.conInfos[iCon];
		i32 iBody0 = con.bodyIds[0];
		i32 iBody1 = con.bodyIds[1];
		i32 iPhase = iCon;  //iBody0; // pseudorandom choice to distribute evenly amongst phases
		iPhase &= params.phaseMask;
		i32 gridCoord[3];
		// is it a dynamic constraint?
		if (params.bodyDynamicFlags[iBody0] && params.bodyDynamicFlags[iBody1])
		{
			const IntVec3& body0Coords = params.bodyGridCoords[iBody0];
			const IntVec3& body1Coords = params.bodyGridCoords[iBody1];
			// for each dimension x,y,z,
			for (i32 i = 0; i < 3; ++i)
			{
				i32 coordMin = d3Min(body0Coords.m_ints[i], body1Coords.m_ints[i]);
				i32 coordMax = d3Max(body0Coords.m_ints[i], body1Coords.m_ints[i]);
				if (coordMin != coordMax)
				{
					Assert(coordMax == coordMin + 1);
					if ((coordMin & 1) == 0)
					{
						iPhase &= ~(1 << i);  // force bit off
					}
					else
					{
						iPhase |= (1 << i);  // force bit on
						iPhase &= params.phaseMask;
					}
				}
				gridCoord[i] = coordMin;
			}
		}
		else
		{
			if (!params.bodyDynamicFlags[iBody0])
			{
				iBody0 = con.bodyIds[1];
			}
			Assert(params.bodyDynamicFlags[iBody0]);
			const IntVec3& body0Coords = params.bodyGridCoords[iBody0];
			// for each dimension x,y,z,
			for (i32 i = 0; i < 3; ++i)
			{
				gridCoord[i] = body0Coords.m_ints[i];
			}
		}
		// calculate chunk coordinates
		i32 chunkCoord[3];
		IntVec3 gridChunkDim = params.gridChunkDim;
		// for each dimension x,y,z,
		for (i32 i = 0; i < 3; ++i)
		{
			i32 coordOffset = (iPhase >> i) & 1;
			chunkCoord[i] = (gridCoord[i] - coordOffset) / 2;
			Clamp(chunkCoord[i], 0, gridChunkDim[i] - 1);
			Assert(chunkCoord[i] < gridChunkDim[i]);
		}
		i32 iBatch = iPhase * params.maxNumBatchesPerPhase + chunkCoord[0] + chunkCoord[1] * gridChunkDim[0] + chunkCoord[2] * gridChunkDim[0] * gridChunkDim[1];
		Assert(iBatch >= 0 && iBatch < params.maxNumBatchesPerPhase * params.numPhases);
		params.constraintBatchIds[iCon] = iBatch;
	}
}

struct AssignConstraintsToGridBatchesLoop : public IParallelForBody
{
	const AssignConstraintsToGridBatchesParams* m_params;

	AssignConstraintsToGridBatchesLoop(const AssignConstraintsToGridBatchesParams& params)
	{
		m_params = &params;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		assignConstraintsToGridBatches(*m_params, iBegin, iEnd);
	}
};

//
// setupSpatialGridBatchesMt -- generate batches using a uniform 3D grid
//
/*

Bodies are treated as 3D points at their center of mass. We only consider dynamic bodies at this stage,
because only dynamic bodies are mutated when a constraint is solved, thus subject to race conditions.

1. Compute a bounding box around all dynamic bodies
2. Compute the maximum extent of all dynamic constraints. Each dynamic constraint is treated as a line segment, and we need the size of
   box that will fully enclose any single dynamic constraint

3. Establish the cell size of our grid, the cell size in each dimension must be at least as large as the dynamic constraints max-extent,
   so that no dynamic constraint can span more than 2 cells of our grid on any axis of the grid. The cell size should be adjusted
   larger in order to keep the total number of cells from being excessively high

Key idea: Given that each constraint spans 1 or 2 grid cells in each dimension, we can handle all constraints by processing
          in chunks of 2x2x2 cells with 8 different 1-cell offsets ((0,0,0),(0,0,1),(0,1,0),(0,1,1),(1,0,0)...).
          For each of the 8 offsets, we create a phase, and for each 2x2x2 chunk with dynamic constraints becomes a batch in that phase.

4. Once the grid is established, we can calculate for each constraint which phase and batch it belongs in.

5. Do a merge small batches on the batches of each phase separately, to try to even out the sizes of batches

Optionally, we can "collapse" one dimension of our 3D grid to turn it into a 2D grid, which reduces the number of phases
to 4. With fewer phases, there are more constraints per phase and this makes it easier to create batches of a useful size.
*/
//
static void setupSpatialGridBatchesMt(
	BatchedConstraints* batchedConstraints,
	AlignedObjectArray<char>* scratchMemory,
	ConstraintArray* constraints,
	const AlignedObjectArray<SolverBody>& bodies,
	i32 minBatchSize,
	i32 maxBatchSize,
	bool use2DGrid)
{
	DRX3D_PROFILE("setupSpatialGridBatchesMt");
	i32k numPhases = 8;
	i32 numConstraints = constraints->size();
	i32 numConstraintRows = constraints->size();

	i32k maxGridChunkCount = 128;
	i32 allocNumBatchesPerPhase = maxGridChunkCount;
	i32 minNumBatchesPerPhase = 16;
	i32 allocNumBatches = allocNumBatchesPerPhase * numPhases;

	Vec3* bodyPositions = NULL;
	bool* bodyDynamicFlags = NULL;
	IntVec3* bodyGridCoords = NULL;
	BatchInfo* batches = NULL;
	i32* batchWork = NULL;
	BatchedConstraintInfo* conInfos = NULL;
	i32* constraintBatchIds = NULL;
	i32* constraintRowBatchIds = NULL;
	{
		PreallocatedMemoryHelper<10> memHelper;
		memHelper.addChunk((uk *)&bodyPositions, sizeof(Vec3) * bodies.size());
		memHelper.addChunk((uk *)&bodyDynamicFlags, sizeof(bool) * bodies.size());
		memHelper.addChunk((uk *)&bodyGridCoords, sizeof(IntVec3) * bodies.size());
		memHelper.addChunk((uk *)&batches, sizeof(BatchInfo) * allocNumBatches);
		memHelper.addChunk((uk *)&batchWork, sizeof(i32) * allocNumBatches);
		memHelper.addChunk((uk *)&conInfos, sizeof(BatchedConstraintInfo) * numConstraints);
		memHelper.addChunk((uk *)&constraintBatchIds, sizeof(i32) * numConstraints);
		memHelper.addChunk((uk *)&constraintRowBatchIds, sizeof(i32) * numConstraintRows);
		size_t scratchSize = memHelper.getSizeToAllocate();
		// if we need to reallocate
		if (static_cast<size_t>(scratchMemory->capacity()) < scratchSize)
		{
			// allocate 6.25% extra to avoid repeated reallocs
			scratchMemory->reserve(scratchSize + scratchSize / 16);
		}
		scratchMemory->resizeNoInitialize(scratchSize);
		tuk memPtr = &scratchMemory->at(0);
		memHelper.setChunkPointers(memPtr);
	}

	numConstraints = initBatchedConstraintInfo(conInfos, constraints);

	// compute bounding box around all dynamic bodies
	// (could be done in parallel)
	Vec3 bboxMin(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
	Vec3 bboxMax = -bboxMin;
	//i32 dynamicBodyCount = 0;
	for (i32 i = 0; i < bodies.size(); ++i)
	{
		const SolverBody& body = bodies[i];
		Vec3 bodyPos = body.getWorldTransform().getOrigin();
		bool isDynamic = (body.internalGetInvMass().x() > Scalar(0));
		bodyPositions[i] = bodyPos;
		bodyDynamicFlags[i] = isDynamic;
		if (isDynamic)
		{
			//dynamicBodyCount++;
			bboxMin.setMin(bodyPos);
			bboxMax.setMax(bodyPos);
		}
	}

	// find max extent of all dynamic constraints
	// (could be done in parallel)
	Vec3 consExtent = findMaxDynamicConstraintExtent(bodyPositions, bodyDynamicFlags, conInfos, numConstraints, bodies.size());

	Vec3 gridExtent = bboxMax - bboxMin;

	gridExtent.setMax(Vec3(Scalar(1), Scalar(1), Scalar(1)));

	Vec3 gridCellSize = consExtent;
	i32 gridDim[3];
	gridDim[0] = i32(1.0 + gridExtent.x() / gridCellSize.x());
	gridDim[1] = i32(1.0 + gridExtent.y() / gridCellSize.y());
	gridDim[2] = i32(1.0 + gridExtent.z() / gridCellSize.z());

	// if we can collapse an axis, it will cut our number of phases in half which could be more efficient
	i32 phaseMask = 7;
	bool collapseAxis = use2DGrid;
	if (collapseAxis)
	{
		// pick the smallest axis to collapse, leaving us with the greatest number of cells in our grid
		i32 iAxisToCollapse = 0;
		i32 axisDim = gridDim[iAxisToCollapse];
		//for each dimension
		for (i32 i = 0; i < 3; ++i)
		{
			if (gridDim[i] < axisDim)
			{
				iAxisToCollapse = i;
				axisDim = gridDim[i];
			}
		}
		// collapse it
		gridCellSize[iAxisToCollapse] = gridExtent[iAxisToCollapse] * 2.0f;
		phaseMask &= ~(1 << iAxisToCollapse);
	}

	i32 numGridChunks = 0;
	IntVec3 gridChunkDim;  // each chunk is 2x2x2 group of cells
	while (true)
	{
		gridDim[0] = i32(1.0 + gridExtent.x() / gridCellSize.x());
		gridDim[1] = i32(1.0 + gridExtent.y() / gridCellSize.y());
		gridDim[2] = i32(1.0 + gridExtent.z() / gridCellSize.z());
		gridChunkDim[0] = d3Max(1, (gridDim[0] + 0) / 2);
		gridChunkDim[1] = d3Max(1, (gridDim[1] + 0) / 2);
		gridChunkDim[2] = d3Max(1, (gridDim[2] + 0) / 2);
		numGridChunks = gridChunkDim[0] * gridChunkDim[1] * gridChunkDim[2];
		float nChunks = float(gridChunkDim[0]) * float(gridChunkDim[1]) * float(gridChunkDim[2]);  // suceptible to integer overflow
		if (numGridChunks <= maxGridChunkCount && nChunks <= maxGridChunkCount)
		{
			break;
		}
		gridCellSize *= 1.25;  // should roughly cut numCells in half
	}
	Assert(numGridChunks <= maxGridChunkCount);
	i32 maxNumBatchesPerPhase = numGridChunks;

	// for each dynamic body, compute grid coords
	Vec3 invGridCellSize = Vec3(1, 1, 1) / gridCellSize;
	// (can be done in parallel)
	for (i32 iBody = 0; iBody < bodies.size(); ++iBody)
	{
		IntVec3& coords = bodyGridCoords[iBody];
		if (bodyDynamicFlags[iBody])
		{
			Vec3 v = (bodyPositions[iBody] - bboxMin) * invGridCellSize;
			coords.m_ints[0] = i32(v.x());
			coords.m_ints[1] = i32(v.y());
			coords.m_ints[2] = i32(v.z());
			Assert(coords.m_ints[0] >= 0 && coords.m_ints[0] < gridDim[0]);
			Assert(coords.m_ints[1] >= 0 && coords.m_ints[1] < gridDim[1]);
			Assert(coords.m_ints[2] >= 0 && coords.m_ints[2] < gridDim[2]);
		}
		else
		{
			coords.m_ints[0] = -1;
			coords.m_ints[1] = -1;
			coords.m_ints[2] = -1;
		}
	}

	for (i32 iPhase = 0; iPhase < numPhases; ++iPhase)
	{
		i32 batchBegin = iPhase * maxNumBatchesPerPhase;
		i32 batchEnd = batchBegin + maxNumBatchesPerPhase;
		for (i32 iBatch = batchBegin; iBatch < batchEnd; ++iBatch)
		{
			BatchInfo& batch = batches[iBatch];
			batch = BatchInfo();
		}
	}

	{
		AssignConstraintsToGridBatchesParams params;
		params.bodyDynamicFlags = bodyDynamicFlags;
		params.bodyGridCoords = bodyGridCoords;
		params.numBodies = bodies.size();
		params.conInfos = conInfos;
		params.constraintBatchIds = constraintBatchIds;
		params.gridChunkDim = gridChunkDim;
		params.maxNumBatchesPerPhase = maxNumBatchesPerPhase;
		params.numPhases = numPhases;
		params.phaseMask = phaseMask;
		bool inParallel = true;
		if (inParallel)
		{
			AssignConstraintsToGridBatchesLoop loop(params);
			i32 grainSize = 250;
			ParallelFor(0, numConstraints, grainSize, loop);
		}
		else
		{
			assignConstraintsToGridBatches(params, 0, numConstraints);
		}
	}
	for (i32 iCon = 0; iCon < numConstraints; ++iCon)
	{
		const BatchedConstraintInfo& con = conInfos[iCon];
		i32 iBatch = constraintBatchIds[iCon];
		BatchInfo& batch = batches[iBatch];
		batch.numConstraints += con.numConstraintRows;
	}

	for (i32 iPhase = 0; iPhase < numPhases; ++iPhase)
	{
		// if phase is legit,
		if (iPhase == (iPhase & phaseMask))
		{
			i32 iBeginBatch = iPhase * maxNumBatchesPerPhase;
			i32 iEndBatch = iBeginBatch + maxNumBatchesPerPhase;
			mergeSmallBatches(batches, iBeginBatch, iEndBatch, minBatchSize, maxBatchSize);
		}
	}
	// all constraints have been assigned a batchId
	updateConstraintBatchIdsForMergesMt(constraintBatchIds, numConstraints, batches, maxNumBatchesPerPhase * numPhases);

	if (numConstraintRows > numConstraints)
	{
		expandConstraintRowsMt(&constraintRowBatchIds[0], &constraintBatchIds[0], &conInfos[0], numConstraints, numConstraintRows);
	}
	else
	{
		constraintRowBatchIds = constraintBatchIds;
	}

	writeOutBatches(batchedConstraints, constraintRowBatchIds, numConstraintRows, batches, batchWork, maxNumBatchesPerPhase, numPhases);
	Assert(batchedConstraints->validate(constraints, bodies));
}

static void setupSingleBatch(
	BatchedConstraints* bc,
	i32 numConstraints)
{
	DRX3D_PROFILE("setupSingleBatch");
	typedef BatchedConstraints::Range Range;

	bc->m_constraintIndices.resize(numConstraints);
	for (i32 i = 0; i < numConstraints; ++i)
	{
		bc->m_constraintIndices[i] = i;
	}

	bc->m_batches.resizeNoInitialize(0);
	bc->m_phases.resizeNoInitialize(0);
	bc->m_phaseOrder.resizeNoInitialize(0);
	bc->m_phaseGrainSize.resizeNoInitialize(0);

	if (numConstraints > 0)
	{
		bc->m_batches.push_back(Range(0, numConstraints));
		bc->m_phases.push_back(Range(0, 1));
		bc->m_phaseOrder.push_back(0);
		bc->m_phaseGrainSize.push_back(1);
	}
}

void BatchedConstraints::setup(
	ConstraintArray* constraints,
	const AlignedObjectArray<SolverBody>& bodies,
	BatchingMethod batchingMethod,
	i32 minBatchSize,
	i32 maxBatchSize,
	AlignedObjectArray<char>* scratchMemory)
{
	if (constraints->size() >= minBatchSize * 4)
	{
		bool use2DGrid = batchingMethod == BATCHING_METHOD_SPATIAL_GRID_2D;
		setupSpatialGridBatchesMt(this, scratchMemory, constraints, bodies, minBatchSize, maxBatchSize, use2DGrid);
		if (s_debugDrawBatches)
		{
			debugDrawAllBatches(this, constraints, bodies);
		}
	}
	else
	{
		setupSingleBatch(this, constraints->size());
	}
}
