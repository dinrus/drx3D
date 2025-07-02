#ifndef DRX3D_MOTIONSTATE_H
#define DRX3D_MOTIONSTATE_H

#include <drx3D/Maths/Linear/Transform2.h>

///The MotionState interface class allows the dynamics world to synchronize
// and interpolate the updated world transforms with graphics
///For optimizations, potentially only moving objects get synchronized
//(using setWorldPosition/setWorldOrientation)
class MotionState
{
public:
	virtual ~MotionState()
	{
	}

	virtual void getWorldTransform(Transform2& worldTrans) const = 0;

	//drx3D only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const Transform2& worldTrans) = 0;
};

#endif  //DRX3D_MOTIONSTATE_H
