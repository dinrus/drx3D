#ifndef D3_GPU_JACOBI_CONTACT_SOLVER_H
#define D3_GPU_JACOBI_CONTACT_SOLVER_H
#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>

class b3TypedConstraint;

struct b3JacobiSolverInfo
{
	i32 m_fixedBodyIndex;

	float m_deltaTime;
	float m_positionDrift;
	float m_positionConstraintCoeff;
	i32 m_numIterations;

	b3JacobiSolverInfo()
		: m_fixedBodyIndex(0),
		  m_deltaTime(1. / 60.f),
		  m_positionDrift(0.005f),
		  m_positionConstraintCoeff(0.99f),
		  m_numIterations(7)
	{
	}
};
class b3GpuJacobiContactSolver
{
protected:
	struct b3GpuJacobiSolverInternalData* m_data;

	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

public:
	b3GpuJacobiContactSolver(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 pairCapacity);
	virtual ~b3GpuJacobiContactSolver();

	void solveContacts(i32 numBodies, cl_mem bodyBuf, cl_mem inertiaBuf, i32 numContacts, cl_mem contactBuf, const struct b3Config& config, i32 static0Index);
	void solveGroupHost(b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numBodies, struct b3Contact4* manifoldPtr, i32 numManifolds, const b3JacobiSolverInfo& solverInfo);
};
#endif  //D3_GPU_JACOBI_CONTACT_SOLVER_H
