#ifndef DRX3D_BOX_BOX_DETECTOR_H
#define DRX3D_BOX_BOX_DETECTOR_H

class BoxShape;
#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>

/// BoxBoxDetector wraps the ODE box-box collision detector
/// re-distributed under the Zlib license with permission from Russell L. Smith
struct BoxBoxDetector : public DiscreteCollisionDetectorInterface
{
	const BoxShape* m_box1;
	const BoxShape* m_box2;

public:
	BoxBoxDetector(const BoxShape* box1, const BoxShape* box2);

	virtual ~BoxBoxDetector(){};

	virtual void getClosestPoints(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw, bool swapResults = false);
};

#endif  //DRX3D_BOX_BOX_DETECTOR_H
