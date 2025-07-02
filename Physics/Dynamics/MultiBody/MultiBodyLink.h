#ifndef DRX3D_MULTIBODY_LINK_H
#define DRX3D_MULTIBODY_LINK_H

#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

enum MultiBodyLinkFlags
{
	DRX3D_MULTIBODYLINKFLAGS_DISABLE_PARENT_COLLISION = 1,
	DRX3D_MULTIBODYLINKFLAGS_DISABLE_ALL_PARENT_COLLISION = 2,
};

//both defines are now permanently enabled
#define DRX3D_MULTIBODYLINK_INCLUDE_PLANAR_JOINTS
#define TEST_SPATIAL_ALGEBRA_LAYER

//
// Various spatial helper functions
//

//namespace {

#include <drx3D/Maths/Linear/SpatialAlgebra.h>

//}

//
// Link struct
//

struct MultibodyLink
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Scalar m_mass;           // mass of link
	Vec3 m_inertiaLocal;  // inertia of link (local frame; diagonal)

	i32 m_parent;  // index of the parent link (assumed to be < index of this link), or -1 if parent is the base link.

	Quat m_zeroRotParentToThis;  // rotates vectors in parent-frame to vectors in local-frame (when q=0). constant.

	Vec3 m_dVector;  // vector from the inboard joint pos to this link's COM. (local frame.) constant.
						  //this is set to zero for planar joint (see also m_eVector comment)

	// m_eVector is constant, but depends on the joint type:
	// revolute, fixed, prismatic, spherical: vector from parent's COM to the pivot point, in PARENT's frame.
	// planar: vector from COM of parent to COM of this link, WHEN Q = 0. (local frame.)
	// todo: fix the planar so it is consistent with the other joints

	Vec3 m_eVector;

	SpatialMotionVector m_absFrameTotVelocity, m_absFrameLocVelocity;

	enum eMultiBodyJointType
	{
		eRevolute = 0,
		ePrismatic = 1,
		eSpherical = 2,
		ePlanar = 3,
		eFixed = 4,
		eInvalid
	};

	// "axis" = spatial joint axis (Mirtich Defn 9 p104). (expressed in local frame.) constant.
	// for prismatic: m_axesTop[0] = zero;
	//                m_axesBottom[0] = unit vector along the joint axis.
	// for revolute: m_axesTop[0] = unit vector along the rotation axis (u);
	//               m_axesBottom[0] = u cross m_dVector (i.e. COM linear motion due to the rotation at the joint)
	//
	// for spherical: m_axesTop[0][1][2] (u1,u2,u3) form a 3x3 identity matrix (3 rotation axes)
	//				  m_axesBottom[0][1][2] cross u1,u2,u3 (i.e. COM linear motion due to the rotation at the joint)
	//
	// for planar: m_axesTop[0] = unit vector along the rotation axis (u); defines the plane of motion
	//			   m_axesTop[1][2] = zero
	//			   m_axesBottom[0] = zero
	//			   m_axesBottom[1][2] = unit vectors along the translational axes on that plane
	SpatialMotionVector m_axes[6];
	void setAxisTop(i32 dof, const Vec3 &axis) { m_axes[dof].m_topVec = axis; }
	void setAxisBottom(i32 dof, const Vec3 &axis)
	{
		m_axes[dof].m_bottomVec = axis;
	}
	void setAxisTop(i32 dof, const Scalar &x, const Scalar &y, const Scalar &z)
	{
		m_axes[dof].m_topVec.setVal(x, y, z);
	}
	void setAxisBottom(i32 dof, const Scalar &x, const Scalar &y, const Scalar &z)
	{
		m_axes[dof].m_bottomVec.setVal(x, y, z);
	}
	const Vec3 &getAxisTop(i32 dof) const { return m_axes[dof].m_topVec; }
	const Vec3 &getAxisBottom(i32 dof) const { return m_axes[dof].m_bottomVec; }

	i32 m_dofOffset, m_cfgOffset;

	Quat m_cachedRotParentToThis;  // rotates vectors in parent frame to vectors in local frame
	Vec3 m_cachedRVector;             // vector from COM of parent to COM of this link, in local frame.
    
    // predicted verstion
    Quat m_cachedRotParentToThis_interpolate;  // rotates vectors in parent frame to vectors in local frame
    Vec3 m_cachedRVector_interpolate;             // vector from COM of parent to COM of this link, in local frame.

	Vec3 m_appliedForce;   // In WORLD frame
	Vec3 m_appliedTorque;  // In WORLD frame

	Vec3 m_appliedConstraintForce;   // In WORLD frame
	Vec3 m_appliedConstraintTorque;  // In WORLD frame

	Scalar m_jointPos[7];
    Scalar m_jointPos_interpolate[7];

	//m_jointTorque is the joint torque applied by the user using 'addJointTorque'.
	//It gets set to zero after each internal stepSimulation call
	Scalar m_jointTorque[6];

	class MultiBodyLinkCollider *m_collider;
	i32 m_flags;

	i32 m_dofCount, m_posVarCount;  //redundant but handy

	eMultiBodyJointType m_jointType;

	struct MultiBodyJointFeedback *m_jointFeedback;

	Transform2 m_cachedWorldTransform;  //this cache is updated when calling MultiBody::forwardKinematics

	tukk m_linkName;   //m_linkName memory needs to be managed by the developer/user!
	tukk m_jointName;  //m_jointName memory needs to be managed by the developer/user!
	ukk m_userPtr;    //m_userPtr ptr needs to be managed by the developer/user!

	Scalar m_jointDamping;      //todo: implement this internally. It is unused for now, it is set by a URDF loader. User can apply manual damping.
	Scalar m_jointFriction;     //todo: implement this internally. It is unused for now, it is set by a URDF loader. User can apply manual friction using a velocity motor.
	Scalar m_jointLowerLimit;   //todo: implement this internally. It is unused for now, it is set by a URDF loader.
	Scalar m_jointUpperLimit;   //todo: implement this internally. It is unused for now, it is set by a URDF loader.
	Scalar m_jointMaxForce;     //todo: implement this internally. It is unused for now, it is set by a URDF loader.
	Scalar m_jointMaxVelocity;  //todo: implement this internally. It is unused for now, it is set by a URDF loader.

	// ctor: set some sensible defaults
	MultibodyLink()
		: m_mass(1),
		  m_parent(-1),
		  m_zeroRotParentToThis(0, 0, 0, 1),
		  m_cachedRotParentToThis(0, 0, 0, 1),
          m_cachedRotParentToThis_interpolate(0, 0, 0, 1),
		  m_collider(0),
		  m_flags(0),
		  m_dofCount(0),
		  m_posVarCount(0),
		  m_jointType(MultibodyLink::eInvalid),
		  m_jointFeedback(0),
		  m_linkName(0),
		  m_jointName(0),
		  m_userPtr(0),
		  m_jointDamping(0),
		  m_jointFriction(0),
		  m_jointLowerLimit(0),
		  m_jointUpperLimit(0),
		  m_jointMaxForce(0),
		  m_jointMaxVelocity(0)
	{
		m_inertiaLocal.setVal(1, 1, 1);
		setAxisTop(0, 0., 0., 0.);
		setAxisBottom(0, 1., 0., 0.);
		m_dVector.setVal(0, 0, 0);
		m_eVector.setVal(0, 0, 0);
		m_cachedRVector.setVal(0, 0, 0);
        m_cachedRVector_interpolate.setVal(0, 0, 0);
		m_appliedForce.setVal(0, 0, 0);
		m_appliedTorque.setVal(0, 0, 0);
		m_appliedConstraintForce.setVal(0, 0, 0);
		m_appliedConstraintTorque.setVal(0, 0, 0);
		//
		m_jointPos[0] = m_jointPos[1] = m_jointPos[2] = m_jointPos[4] = m_jointPos[5] = m_jointPos[6] = 0.f;
		m_jointPos[3] = 1.f;  //"quat.w"
		m_jointTorque[0] = m_jointTorque[1] = m_jointTorque[2] = m_jointTorque[3] = m_jointTorque[4] = m_jointTorque[5] = 0.f;
		m_cachedWorldTransform.setIdentity();
	}

	// routine to update m_cachedRotParentToThis and m_cachedRVector
	void updateCacheMultiDof(Scalar *pq = 0)
	{
        Scalar *pJointPos = (pq ? pq : &m_jointPos[0]);
        Quat& cachedRot = m_cachedRotParentToThis;
        Vec3& cachedVector = m_cachedRVector;
		switch (m_jointType)
		{
			case eRevolute:
			{
				cachedRot = Quat(getAxisTop(0), -pJointPos[0]) * m_zeroRotParentToThis;
				cachedVector = m_dVector + quatRotate(m_cachedRotParentToThis, m_eVector);

				break;
			}
			case ePrismatic:
			{
				// m_cachedRotParentToThis never changes, so no need to update
				cachedVector = m_dVector + quatRotate(m_cachedRotParentToThis, m_eVector) + pJointPos[0] * getAxisBottom(0);

				break;
			}
			case eSpherical:
			{
				cachedRot = Quat(pJointPos[0], pJointPos[1], pJointPos[2], -pJointPos[3]) * m_zeroRotParentToThis;
				cachedVector = m_dVector + quatRotate(cachedRot, m_eVector);

				break;
			}
			case ePlanar:
			{
				cachedRot = Quat(getAxisTop(0), -pJointPos[0]) * m_zeroRotParentToThis;
				cachedVector = quatRotate(Quat(getAxisTop(0), -pJointPos[0]), pJointPos[1] * getAxisBottom(1) + pJointPos[2] * getAxisBottom(2)) + quatRotate(cachedRot, m_eVector);

				break;
			}
			case eFixed:
			{
				cachedRot = m_zeroRotParentToThis;
				cachedVector = m_dVector + quatRotate(cachedRot, m_eVector);

				break;
			}
			default:
			{
				//invalid type
				Assert(0);
			}
		}
        m_cachedRotParentToThis_interpolate = m_cachedRotParentToThis;
        m_cachedRVector_interpolate = m_cachedRVector;
	}
    
    void updateInterpolationCacheMultiDof()
    {
        Scalar *pJointPos = &m_jointPos_interpolate[0];
        
        Quat& cachedRot = m_cachedRotParentToThis_interpolate;
        Vec3& cachedVector = m_cachedRVector_interpolate;
        switch (m_jointType)
        {
            case eRevolute:
            {
                cachedRot = Quat(getAxisTop(0), -pJointPos[0]) * m_zeroRotParentToThis;
                cachedVector = m_dVector + quatRotate(m_cachedRotParentToThis, m_eVector);
                
                break;
            }
            case ePrismatic:
            {
                // m_cachedRotParentToThis never changes, so no need to update
                cachedVector = m_dVector + quatRotate(m_cachedRotParentToThis, m_eVector) + pJointPos[0] * getAxisBottom(0);
                
                break;
            }
            case eSpherical:
            {
                cachedRot = Quat(pJointPos[0], pJointPos[1], pJointPos[2], -pJointPos[3]) * m_zeroRotParentToThis;
                cachedVector = m_dVector + quatRotate(cachedRot, m_eVector);
                
                break;
            }
            case ePlanar:
            {
                cachedRot = Quat(getAxisTop(0), -pJointPos[0]) * m_zeroRotParentToThis;
                cachedVector = quatRotate(Quat(getAxisTop(0), -pJointPos[0]), pJointPos[1] * getAxisBottom(1) + pJointPos[2] * getAxisBottom(2)) + quatRotate(cachedRot, m_eVector);
                
                break;
            }
            case eFixed:
            {
                cachedRot = m_zeroRotParentToThis;
                cachedVector = m_dVector + quatRotate(cachedRot, m_eVector);
                
                break;
            }
            default:
            {
                //invalid type
                Assert(0);
            }
        }
    }

 

};

#endif  //DRX3D_MULTIBODY_LINK_H
