#ifndef LW_COLLISION_C_API_H
#define LW_COLLISION_C_API_H

#include <drxtypes.h>

#define PL_DECLARE_HANDLE(name) \
	typedef struct name##__     \
	{                           \
		i32 unused;             \
	} * name

#ifdef DRX3D_USE_DOUBLE_PRECISION
typedef double plReal;
#else
typedef float plReal;
#endif

typedef plReal plVector3[3];
typedef plReal plQuat[4];

#ifdef __cplusplus
extern "C"
{
#endif

	/**     Particular collision SDK (C-API) */
	PL_DECLARE_HANDLE(plCollisionSdkHandle);

	/**     Collision world, belonging to some collision SDK (C-API)*/
	PL_DECLARE_HANDLE(plCollisionWorldHandle);

	/** Collision object that can be part of a collision World (C-API)*/
	PL_DECLARE_HANDLE(plCollisionObjectHandle);

	/**     Collision Shape/Geometry, property of a collision object (C-API)*/
	PL_DECLARE_HANDLE(plCollisionShapeHandle);

	/* Collision SDK */

	extern plCollisionSdkHandle plCreateBullet2CollisionSdk();

#ifndef DISABLE_REAL_TIME_BULLET3_COLLISION_SDK
	extern plCollisionSdkHandle plCreateRealTimeBullet3CollisionSdk();
#endif  //DISABLE_REAL_TIME_BULLET3_COLLISION_SDK

	//	extern plCollisionSdkHandle plCreateCustomCollisionSdk();

	extern void plDeleteCollisionSdk(plCollisionSdkHandle collisionSdkHandle);

	//extern i32 plGetSdkWorldCreationIntParameter();
	//extern i32 plSetSdkWorldCreationIntParameter(i32 newValue);

	/* Collision World */

	extern plCollisionWorldHandle plCreateCollisionWorld(plCollisionSdkHandle collisionSdkHandle, i32 maxNumObjsCapacity, i32 maxNumShapesCapacity, i32 maxNumPairsCapacity);
	extern void plDeleteCollisionWorld(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle world);

	extern void plAddCollisionObject(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle world, plCollisionObjectHandle object);
	extern void plRemoveCollisionObject(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle world, plCollisionObjectHandle object);

	/* Collision Object  */

	extern plCollisionObjectHandle plCreateCollisionObject(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, uk userPointer, i32 userIndex, plCollisionShapeHandle cshape, plVector3 startPosition, plQuat startOrientation);
	extern void plDeleteCollisionObject(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, plCollisionObjectHandle body);
	extern void plSetCollisionObjectTransform(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, plCollisionObjectHandle objHandle, plVector3 startPosition, plQuat startOrientation);

	/* Collision Shape definition */

	extern plCollisionShapeHandle plCreateSphereShape(plCollisionSdkHandle sdk, plCollisionWorldHandle worldHandle, plReal radius);
	extern plCollisionShapeHandle plCreateCapsuleShape(plCollisionSdkHandle sdk, plCollisionWorldHandle worldHandle, plReal radius, plReal height, i32 capsuleAxis);
	extern plCollisionShapeHandle plCreatePlaneShape(plCollisionSdkHandle sdk, plCollisionWorldHandle worldHandle,
													 plReal planeNormalX,
													 plReal planeNormalY,
													 plReal planeNormalZ,
													 plReal planeConstant);
	extern plCollisionShapeHandle plCreateCompoundShape(plCollisionSdkHandle sdk, plCollisionWorldHandle worldHandle);
	extern void plAddChildShape(plCollisionSdkHandle collisionSdkHandle, plCollisionWorldHandle worldHandle, plCollisionShapeHandle compoundShape, plCollisionShapeHandle childShape, plVector3 childPos, plQuat childOrn);

	extern void plDeleteShape(plCollisionSdkHandle collisionSdkHandle, plCollisionWorldHandle worldHandle, plCollisionShapeHandle shape);

	/* Contact Results */

	struct lwContactPoint
	{
		plVector3 m_ptOnAWorld;
		plVector3 m_ptOnBWorld;
		plVector3 m_normalOnB;
		plReal m_distance;
	};

	/* Collision Filtering */
	typedef void (*plNearCallback)(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, uk userData,
								   plCollisionObjectHandle objA, plCollisionObjectHandle objB);

	/* Collision Queries */
	extern i32 plCollide(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, plCollisionObjectHandle colA, plCollisionObjectHandle colB,
						 lwContactPoint* pointsOut, i32 pointCapacity);

	extern void plWorldCollide(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle world,
							   plNearCallback filter, uk userData);

#ifdef __cplusplus
}
#endif

#endif  //LW_COLLISION_C_API_H
