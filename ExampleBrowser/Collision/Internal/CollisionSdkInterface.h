#ifndef COLLISION_SDK_INTERFACE_H
#define COLLISION_SDK_INTERFACE_H

#include "../CollisionSdkC_Api.h"

class CollisionSdkInterface
{
public:
	virtual ~CollisionSdkInterface()
	{
	}

	virtual plCollisionWorldHandle createCollisionWorld(i32 maxNumObjsCapacity, i32 maxNumShapesCapacity, i32 maxNumPairsCapacity) = 0;

	virtual void deleteCollisionWorld(plCollisionWorldHandle worldHandle) = 0;

	virtual plCollisionShapeHandle createSphereShape(plCollisionWorldHandle worldHandle, plReal radius) = 0;

	virtual plCollisionShapeHandle createPlaneShape(plCollisionWorldHandle worldHandle,
													plReal planeNormalX,
													plReal planeNormalY,
													plReal planeNormalZ,
													plReal planeConstant) = 0;

	virtual plCollisionShapeHandle createCapsuleShape(plCollisionWorldHandle worldHandle,
													  plReal radius,
													  plReal height,
													  i32 capsuleAxis) = 0;

	virtual plCollisionShapeHandle createCompoundShape(plCollisionWorldHandle worldHandle) = 0;
	virtual void addChildShape(plCollisionWorldHandle worldHandle, plCollisionShapeHandle compoundShape, plCollisionShapeHandle childShape, plVector3 childPos, plQuat childOrn) = 0;

	virtual void deleteShape(plCollisionWorldHandle worldHandle, plCollisionShapeHandle shape) = 0;

	virtual void addCollisionObject(plCollisionWorldHandle world, plCollisionObjectHandle object) = 0;
	virtual void removeCollisionObject(plCollisionWorldHandle world, plCollisionObjectHandle object) = 0;

	virtual plCollisionObjectHandle createCollisionObject(plCollisionWorldHandle worldHandle, uk userPointer, i32 userIndex, plCollisionShapeHandle cshape,
														  plVector3 startPosition, plQuat startOrientation) = 0;
	virtual void deleteCollisionObject(plCollisionObjectHandle body) = 0;
	virtual void setCollisionObjectTransform(plCollisionWorldHandle world, plCollisionObjectHandle body,
											 plVector3 position, plQuat orientation) = 0;

	virtual i32 collide(plCollisionWorldHandle world, plCollisionObjectHandle colA, plCollisionObjectHandle colB,
						lwContactPoint* pointsOut, i32 pointCapacity) = 0;

	virtual void collideWorld(plCollisionWorldHandle world,
							  plNearCallback filter, uk userData) = 0;
};

#endif  //COLLISION_SDK_INTERFACE_H
