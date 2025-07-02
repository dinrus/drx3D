#ifndef BULLET2_COLLISION_SDK_H
#define BULLET2_COLLISION_SDK_H

#include "CollisionSdkInterface.h"

class Bullet2CollisionSdk : public CollisionSdkInterface
{
	struct Bullet2CollisionSdkInternalData* m_internalData;

public:
	Bullet2CollisionSdk();

	virtual ~Bullet2CollisionSdk();

	virtual plCollisionWorldHandle createCollisionWorld(i32 maxNumObjsCapacity, i32 maxNumShapesCapacity, i32 maxNumPairsCapacity);

	virtual void deleteCollisionWorld(plCollisionWorldHandle worldHandle);

	virtual plCollisionShapeHandle createSphereShape(plCollisionWorldHandle worldHandle, plReal radius);

	virtual plCollisionShapeHandle createPlaneShape(plCollisionWorldHandle worldHandle,
													plReal planeNormalX,
													plReal planeNormalY,
													plReal planeNormalZ,
													plReal planeConstant);

	virtual plCollisionShapeHandle createCapsuleShape(plCollisionWorldHandle worldHandle,
													  plReal radius,
													  plReal height,
													  i32 capsuleAxis);

	virtual plCollisionShapeHandle createCompoundShape(plCollisionWorldHandle worldHandle);
	virtual void addChildShape(plCollisionWorldHandle worldHandle, plCollisionShapeHandle compoundShape, plCollisionShapeHandle childShape, plVector3 childPos, plQuat childOrn);

	virtual void deleteShape(plCollisionWorldHandle worldHandle, plCollisionShapeHandle shape);

	virtual void addCollisionObject(plCollisionWorldHandle world, plCollisionObjectHandle object);
	virtual void removeCollisionObject(plCollisionWorldHandle world, plCollisionObjectHandle object);

	virtual plCollisionObjectHandle createCollisionObject(plCollisionWorldHandle worldHandle, uk userPointer, i32 userIndex, plCollisionShapeHandle cshape,
														  plVector3 startPosition, plQuat startOrientation);
	virtual void deleteCollisionObject(plCollisionObjectHandle body);
	virtual void setCollisionObjectTransform(plCollisionWorldHandle world, plCollisionObjectHandle body,
											 plVector3 position, plQuat orientation);

	virtual i32 collide(plCollisionWorldHandle world, plCollisionObjectHandle colA, plCollisionObjectHandle colB,
						lwContactPoint* pointsOut, i32 pointCapacity);

	virtual void collideWorld(plCollisionWorldHandle world,
							  plNearCallback filter, uk userData);

	static plCollisionSdkHandle createBullet2SdkHandle();
};

#endif  //BULLET2_COLLISION_SDK_H
