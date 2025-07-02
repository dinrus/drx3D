#ifndef D3_CPU_NARROWPHASE_H
#define D3_CPU_NARROWPHASE_H

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>
#include <drx3D/Common/shared/b3Int4.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>

class b3CpuNarrowPhase
{
protected:
	struct b3CpuNarrowPhaseInternalData* m_data;
	i32 m_acceleratedCompanionShapeIndex;
	i32 m_planeBodyIndex;
	i32 m_static0Index;

	i32 registerConvexHullShapeInternal(class b3ConvexUtility* convexPtr, b3Collidable& col);
	i32 registerConcaveMeshShape(b3AlignedObjectArray<b3Vec3>* vertices, b3AlignedObjectArray<i32>* indices, b3Collidable& col, const float* scaling);

public:
	b3CpuNarrowPhase(const struct b3Config& config);

	virtual ~b3CpuNarrowPhase(void);

	i32 registerSphereShape(float radius);
	i32 registerPlaneShape(const b3Vec3& planeNormal, float planeConstant);

	i32 registerCompoundShape(b3AlignedObjectArray<b3GpuChildShape>* childShapes);
	i32 registerFace(const b3Vec3& faceNormal, float faceConstant);

	i32 registerConcaveMesh(b3AlignedObjectArray<b3Vec3>* vertices, b3AlignedObjectArray<i32>* indices, const float* scaling);

	//do they need to be merged?

	i32 registerConvexHullShape(b3ConvexUtility* utilPtr);
	i32 registerConvexHullShape(const float* vertices, i32 strideInBytes, i32 numVertices, const float* scaling);

	//i32 registerRigidBody(i32 collidableIndex, float mass, const float* position, const float* orientation, const float* aabbMin, const float* aabbMax,bool writeToGpu);
	void setObjectTransform2(const float* position, const float* orientation, i32 bodyIndex);

	void writeAllBodiesToGpu();
	void reset();
	void readbackAllBodiesToCpu();
	bool getObjectTransform2FromCpu(float* position, float* orientation, i32 bodyIndex) const;

	void setObjectTransform2Cpu(float* position, float* orientation, i32 bodyIndex);
	void setObjectVelocityCpu(float* linVel, float* angVel, i32 bodyIndex);

	//virtual void computeContacts(cl_mem broadphasePairs, i32 numBroadphasePairs, cl_mem aabbsWorldSpace, i32 numObjects);
	virtual void computeContacts(b3AlignedObjectArray<b3Int4>& pairs, b3AlignedObjectArray<b3Aabb>& aabbsWorldSpace, b3AlignedObjectArray<b3RigidBodyData>& bodies);

	const struct b3RigidBodyData* getBodiesCpu() const;
	//struct b3RigidBodyData* getBodiesCpu();

	i32 getNumBodiesGpu() const;

	i32 getNumBodyInertiasGpu() const;

	const struct b3Collidable* getCollidablesCpu() const;
	i32 getNumCollidablesGpu() const;

	/*const struct b3Contact4* getContactsCPU() const;

	
	i32	getNumContactsGpu() const;
	*/

	const b3AlignedObjectArray<b3Contact4Data>& getContacts() const;

	i32 getNumRigidBodies() const;

	i32 allocateCollidable();

	i32 getStatic0Index() const
	{
		return m_static0Index;
	}
	b3Collidable& getCollidableCpu(i32 collidableIndex);
	const b3Collidable& getCollidableCpu(i32 collidableIndex) const;

	const b3CpuNarrowPhaseInternalData* getInternalData() const
	{
		return m_data;
	}

	const struct b3Aabb& getLocalSpaceAabb(i32 collidableIndex) const;
};

#endif  //D3_CPU_NARROWPHASE_H
