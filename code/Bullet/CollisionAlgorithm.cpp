#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>

CollisionAlgorithm::CollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
{
	m_dispatcher = ci.m_dispatcher1;
}
