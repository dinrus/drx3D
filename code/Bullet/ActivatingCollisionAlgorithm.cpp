#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

ActivatingCollisionAlgorithm::ActivatingCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
	: CollisionAlgorithm(ci)
//,
//m_colObj0(0),
//m_colObj1(0)
{
}
ActivatingCollisionAlgorithm::ActivatingCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper*, const CollisionObject2Wrapper*)
	: CollisionAlgorithm(ci)
//,
//m_colObj0(0),
//m_colObj1(0)
{
	//	if (ci.m_dispatcher1->needsCollision(colObj0,colObj1))
	//	{
	//		m_colObj0 = colObj0;
	//		m_colObj1 = colObj1;
	//
	//		m_colObj0->activate();
	//		m_colObj1->activate();
	//	}
}

ActivatingCollisionAlgorithm::~ActivatingCollisionAlgorithm()
{
	//		m_colObj0->activate();
	//		m_colObj1->activate();
}
