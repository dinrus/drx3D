#ifndef DRX3D_DEFAULT_MOTION_STATE_H
#define DRX3D_DEFAULT_MOTION_STATE_H

#include <drx3D/Maths/Linear/MotionState.h>

///The DefaultMotionState provides a common implementation to synchronize world transforms with offsets.
ATTRIBUTE_ALIGNED16(struct)
DefaultMotionState : public MotionState
{
	Transform2 m_graphicsWorldTrans;
	Transform2 m_centerOfMassOffset;
	Transform2 m_startWorldTrans;
	uk m_userPointer;

	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	DefaultMotionState(const Transform2& startTrans = Transform2::getIdentity(), const Transform2& centerOfMassOffset = Transform2::getIdentity())
		: m_graphicsWorldTrans(startTrans),
		  m_centerOfMassOffset(centerOfMassOffset),
		  m_startWorldTrans(startTrans),
		  m_userPointer(0)

	{
	}

	///synchronizes world transform from user to physics
	virtual void getWorldTransform(Transform2 & centerOfMassWorldTrans) const
	{
		centerOfMassWorldTrans = m_graphicsWorldTrans * m_centerOfMassOffset.inverse();
	}

	///synchronizes world transform from physics to user
	///drx3D only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const Transform2& centerOfMassWorldTrans)
	{
		m_graphicsWorldTrans = centerOfMassWorldTrans * m_centerOfMassOffset;
	}
};

#endif  //DRX3D_DEFAULT_MOTION_STATE_H
