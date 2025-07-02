#ifndef DRX3D_WHEEL_INFO_H
#define DRX3D_WHEEL_INFO_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>

class RigidBody;

struct WheelInfoConstructionInfo
{
	Vec3 m_chassisConnectionCS;
	Vec3 m_wheelDirectionCS;
	Vec3 m_wheelAxleCS;
	Scalar m_suspensionRestLength;
	Scalar m_maxSuspensionTravelCm;
	Scalar m_wheelRadius;

	Scalar m_suspensionStiffness;
	Scalar m_wheelsDampingCompression;
	Scalar m_wheelsDampingRelaxation;
	Scalar m_frictionSlip;
	Scalar m_maxSuspensionForce;
	bool m_bIsFrontWheel;
};

/// btWheelInfo contains information per wheel about friction and suspension.
struct WheelInfo
{
	struct RaycastInfo
	{
		//set by raycaster
		Vec3 m_contactNormalWS;  //contactnormal
		Vec3 m_contactPointWS;   //raycast hitpoint
		Scalar m_suspensionLength;
		Vec3 m_hardPointWS;       //raycast starting point
		Vec3 m_wheelDirectionWS;  //direction in worldspace
		Vec3 m_wheelAxleWS;       // axle in worldspace
		bool m_isInContact;
		uk m_groundObject;  //could be general uk ptr
	};

	RaycastInfo m_raycastInfo;

	Transform2 m_worldTransform;

	Vec3 m_chassisConnectionPointCS;  //const
	Vec3 m_wheelDirectionCS;          //const
	Vec3 m_wheelAxleCS;               // const or modified by steering
	Scalar m_suspensionRestLength1;      //const
	Scalar m_maxSuspensionTravelCm;
	Scalar getSuspensionRestLength() const;
	Scalar m_wheelsRadius;              //const
	Scalar m_suspensionStiffness;       //const
	Scalar m_wheelsDampingCompression;  //const
	Scalar m_wheelsDampingRelaxation;   //const
	Scalar m_frictionSlip;
	Scalar m_steering;
	Scalar m_rotation;
	Scalar m_deltaRotation;
	Scalar m_rollInfluence;
	Scalar m_maxSuspensionForce;

	Scalar m_engineForce;

	Scalar m_brake;

	bool m_bIsFrontWheel;

	uk m_clientInfo;  //can be used to store pointer to sync transforms...

	WheelInfo() {}

	WheelInfo(WheelInfoConstructionInfo& ci)

	{
		m_suspensionRestLength1 = ci.m_suspensionRestLength;
		m_maxSuspensionTravelCm = ci.m_maxSuspensionTravelCm;

		m_wheelsRadius = ci.m_wheelRadius;
		m_suspensionStiffness = ci.m_suspensionStiffness;
		m_wheelsDampingCompression = ci.m_wheelsDampingCompression;
		m_wheelsDampingRelaxation = ci.m_wheelsDampingRelaxation;
		m_chassisConnectionPointCS = ci.m_chassisConnectionCS;
		m_wheelDirectionCS = ci.m_wheelDirectionCS;
		m_wheelAxleCS = ci.m_wheelAxleCS;
		m_frictionSlip = ci.m_frictionSlip;
		m_steering = Scalar(0.);
		m_engineForce = Scalar(0.);
		m_rotation = Scalar(0.);
		m_deltaRotation = Scalar(0.);
		m_brake = Scalar(0.);
		m_rollInfluence = Scalar(0.1);
		m_bIsFrontWheel = ci.m_bIsFrontWheel;
		m_maxSuspensionForce = ci.m_maxSuspensionForce;
	}

	void updateWheel(const RigidBody& chassis, RaycastInfo& raycastInfo);

	Scalar m_clippedInvContactDotSuspension;
	Scalar m_suspensionRelativeVelocity;
	//calculated by suspension
	Scalar m_wheelsSuspensionForce;
	Scalar m_skidInfo;
};

#endif  //DRX3D_WHEEL_INFO_H
