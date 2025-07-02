#ifndef DRX3D_MULTIBODY_JOINT_FEEDBACK_H
#define DRX3D_MULTIBODY_JOINT_FEEDBACK_H

#include <drx3D/Maths/Linear/SpatialAlgebra.h>

struct MultiBodyJointFeedback
{
	SpatialForceVector m_reactionForces;
};

#endif  //DRX3D_MULTIBODY_JOINT_FEEDBACK_H
