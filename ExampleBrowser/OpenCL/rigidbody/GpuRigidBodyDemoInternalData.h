#ifndef GPU_RIGIDBODY_INTERNAL_DATA_H
#define GPU_RIGIDBODY_INTERNAL_DATA_H

#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3OpenCLArray.h"
#include <drx3D/Common/b3Vec3.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"

struct GpuRigidBodyDemoInternalData
{
	cl_kernel m_copyTransformsToVBOKernel;

	b3OpenCLArray<b3Vec4>* m_instancePosOrnColor;

	class b3GpuRigidBodyPipeline* m_rigidBodyPipeline;

	class b3GpuNarrowPhase* m_np;
	class b3GpuBroadphaseInterface* m_bp;
	struct b3DynamicBvhBroadphase* m_broadphaseDbvt;

	b3Vec3 m_pickPivotInA;
	b3Vec3 m_pickPivotInB;
	float m_pickDistance;
	i32 m_pickBody;
	i32 m_pickConstraint;

	i32 m_altPressed;
	i32 m_controlPressed;

	i32 m_pickFixedBody;
	i32 m_pickGraphicsShapeIndex;
	i32 m_pickGraphicsShapeInstance;
	b3Config m_config;
	GUIHelperInterface* m_guiHelper;

	GpuRigidBodyDemoInternalData()
		: m_instancePosOrnColor(0),
		  m_copyTransformsToVBOKernel(0),
		  m_rigidBodyPipeline(0),
		  m_np(0),
		  m_bp(0),
		  m_broadphaseDbvt(0),
		  m_pickConstraint(-1),
		  m_pickFixedBody(-1),
		  m_pickGraphicsShapeIndex(-1),
		  m_pickGraphicsShapeInstance(-1),
		  m_pickBody(-1),
		  m_altPressed(0),
		  m_controlPressed(0),
		  m_guiHelper(0)

	{
	}
};

#endif  //GPU_RIGIDBODY_INTERNAL_DATA_H
