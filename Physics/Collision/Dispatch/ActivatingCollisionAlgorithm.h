#ifndef __DRX3D_ACTIVATING_COLLISION_ALGORITHM_H
#define __DRX3D_ACTIVATING_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>

///This class is not enabled yet (work-in-progress) to more aggressively activate objects.
class ActivatingCollisionAlgorithm : public CollisionAlgorithm
{
	//	CollisionObject2* m_colObj0;
	//	CollisionObject2* m_colObj1;

protected:
	ActivatingCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci);

	ActivatingCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

public:
	virtual ~ActivatingCollisionAlgorithm();
};
#endif  //__DRX3D_ACTIVATING_COLLISION_ALGORITHM_H
