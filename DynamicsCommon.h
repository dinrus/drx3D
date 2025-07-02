#ifndef DRX3D_DYNAMICS_COMMON_H
#define DRX3D_DYNAMICS_COMMON_H

///Common headerfile includes for drx3D Dynamics, including Collision Detection
#include "CollisionCommon.h"

#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>

#include <drx3D/Physics/Dynamics/SimpleDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

#include <drx3D/Physics/Dynamics/ConstraintSolver/Point2PointConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/HingeConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ConeTwistConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SliderConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpringConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/UniversalConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Hinge2Constraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/GearConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/FixedConstraint.h>

#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>

///Vehicle simulation, with wheel contact simulated by raycasts
#include <drx3D/Physics/Dynamics/Vehicle/RaycastVehicle.h>

#endif  //DRX3D_DYNAMICS_COMMON_H
