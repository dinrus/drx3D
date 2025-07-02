#ifndef DRX3D_SOFTBODY_RIGIDBODY_COLLISION_CONFIGURATION
#define DRX3D_SOFTBODY_RIGIDBODY_COLLISION_CONFIGURATION

#include <drx3D/Physics/Collision/Dispatch/DefaultCollisionConfiguration.h>

class VoronoiSimplexSolver;
class GjkEpaPenetrationDepthSolver;

//SoftBodyRigidBodyCollisionConfiguration add softbody interaction on top of DefaultCollisionConfiguration
class SoftBodyRigidBodyCollisionConfiguration : public DefaultCollisionConfiguration
{
	//default CreationFunctions, filling the m_doubleDispatch table
	CollisionAlgorithmCreateFunc* m_softSoftCreateFunc;
	CollisionAlgorithmCreateFunc* m_softRigidConvexCreateFunc;
	CollisionAlgorithmCreateFunc* m_swappedSoftRigidConvexCreateFunc;
	CollisionAlgorithmCreateFunc* m_softRigidConcaveCreateFunc;
	CollisionAlgorithmCreateFunc* m_swappedSoftRigidConcaveCreateFunc;

public:
	SoftBodyRigidBodyCollisionConfiguration(const DefaultCollisionConstructionInfo& constructionInfo = DefaultCollisionConstructionInfo());

	virtual ~SoftBodyRigidBodyCollisionConfiguration();

	///creation of soft-soft and soft-rigid, and otherwise fallback to base class implementation
	virtual CollisionAlgorithmCreateFunc* getCollisionAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1);
};

#endif  //DRX3D_SOFTBODY_RIGIDBODY_COLLISION_CONFIGURATION
