#ifndef _URDF2DRX3D_H
#define _URDF2DRX3D_H
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <string>
#include "URDFJointTypes.h"  //for UrdfMaterialColor cache

class Vec3;
class Transform2;
class MultiBodyDynamicsWorld;
class DiscreteDynamicsWorld;
class Transform2;

class URDFImporterInterface;
class MultiBodyCreationInterface;



struct UrdfVisualShapeCache
{
	AlignedObjectArray<UrdfMaterialColor> m_cachedUrdfLinkColors;
	AlignedObjectArray<i32> m_cachedUrdfLinkVisualShapeIndices;
};
//#define USE_DISCRETE_DYNAMICS_WORLD
#ifdef USE_DISCRETE_DYNAMICS_WORLD
	void ConvertURDF2Bullet(const URDFImporterInterface& u2b,
						MultiBodyCreationInterface& creationCallback,
						const Transform2& rootTransformInWorldSpace,
						DiscreteDynamicsWorld* world,
						bool createMultiBody,
						tukk pathPrefix,
						i32 flags = 0,
						UrdfVisualShapeCache* cachedLinkGraphicsShapes = 0);

#else
void ConvertURDF2Bullet(const URDFImporterInterface& u2b,
						MultiBodyCreationInterface& creationCallback,
						const Transform2& rootTransformInWorldSpace,
						MultiBodyDynamicsWorld* world,
						bool createMultiBody,
						tukk pathPrefix,
						i32 flags = 0,
						UrdfVisualShapeCache* cachedLinkGraphicsShapes = 0);
#endif
#endif  //_URDF2DRX3D_H
