/*
 * PURPOSE:
 *   Class representing an articulated rigid body. Stores the body's
 *   current state, allows forces and torques to be set, handles
 *   timestepping and implements MultiBody's algorithm.
 */

#ifndef DRX3D_MULTIBODY_H
#define DRX3D_MULTIBODY_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///serialization data, don't change them if you are not familiar with the details of the serialization mechanisms
#ifdef DRX3D_USE_DOUBLE_PRECISION
#define MultiBodyData MultiBodyDoubleData
#define MultiBodyDataName "MultiBodyDoubleData"
#define MultiBodyLinkData MultiBodyLinkDoubleData
#define MultiBodyLinkDataName "MultiBodyLinkDoubleData"
#else
#define MultiBodyData MultiBodyFloatData
#define MultiBodyDataName "MultiBodyFloatData"
#define MultiBodyLinkData MultiBodyLinkFloatData
#define MultiBodyLinkDataName "MultiBodyLinkFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLink.h>

class MultiBodyLinkCollider;

ATTRIBUTE_ALIGNED16(class)
MultiBody
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	//
	// initialization
	//

	MultiBody(i32 n_links,               // NOT including the base
				Scalar mass,             // mass of base
				const Vec3 &inertia,  // inertia of base, in base frame; assumed diagonal
				bool fixedBase,            // whether the base is fixed (true) or can move (false)
				bool canSleep, bool deprecatedMultiDof = true);

	virtual ~MultiBody();

	//note: fixed link collision with parent is always disabled
	void setupFixed(i32 i, //linkIndex
					Scalar mass,
					const Vec3 &inertia,
					i32 parent,
					const Quat &rotParentToThis,
					const Vec3 &parentComToThisPivotOffset,
					const Vec3 &thisPivotToThisComOffset, bool deprecatedDisableParentCollision = true);

	void setupPrismatic(i32 i,
						Scalar mass,
						const Vec3 &inertia,
						i32 parent,
						const Quat &rotParentToThis,
						const Vec3 &jointAxis,
						const Vec3 &parentComToThisPivotOffset,
						const Vec3 &thisPivotToThisComOffset,
						bool disableParentCollision);

	void setupRevolute(i32 i,  // 0 to num_links-1
					   Scalar mass,
					   const Vec3 &inertia,
					   i32 parentIndex,
					   const Quat &rotParentToThis,          // rotate points in parent frame to this frame, when q = 0
					   const Vec3 &jointAxis,                   // in my frame
					   const Vec3 &parentComToThisPivotOffset,  // vector from parent COM to joint axis, in PARENT frame
					   const Vec3 &thisPivotToThisComOffset,    // vector from joint axis to my COM, in MY frame
					   bool disableParentCollision = false);

	void setupSpherical(i32 i,  // linkIndex, 0 to num_links-1
						Scalar mass,
						const Vec3 &inertia,
						i32 parent,
						const Quat &rotParentToThis,          // rotate points in parent frame to this frame, when q = 0
						const Vec3 &parentComToThisPivotOffset,  // vector from parent COM to joint axis, in PARENT frame
						const Vec3 &thisPivotToThisComOffset,    // vector from joint axis to my COM, in MY frame
						bool disableParentCollision = false);

	void setupPlanar(i32 i,  // 0 to num_links-1
					 Scalar mass,
					 const Vec3 &inertia,
					 i32 parent,
					 const Quat &rotParentToThis,  // rotate points in parent frame to this frame, when q = 0
					 const Vec3 &rotationAxis,
					 const Vec3 &parentComToThisComOffset,  // vector from parent COM to this COM, in PARENT frame
					 bool disableParentCollision = false);

	const MultibodyLink &getLink(i32 index) const
	{
		return m_links[index];
	}

	MultibodyLink &getLink(i32 index)
	{
		return m_links[index];
	}

	void setBaseCollider(MultiBodyLinkCollider * collider)  //collider can be NULL to disable collision for the base
	{
		m_baseCollider = collider;
	}
	const MultiBodyLinkCollider *getBaseCollider() const
	{
		return m_baseCollider;
	}
	MultiBodyLinkCollider *getBaseCollider()
	{
		return m_baseCollider;
	}

	const MultiBodyLinkCollider *getLinkCollider(i32 index) const
	{
		if (index >= 0 && index < getNumLinks())
		{
			return getLink(index).m_collider;
		}
		return 0;
	}

	MultiBodyLinkCollider *getLinkCollider(i32 index)
	{
		if (index >= 0 && index < getNumLinks())
		{
			return getLink(index).m_collider;
		}
		return 0;
	}

	//
	// get parent
	// input: link num from 0 to num_links-1
	// output: link num from 0 to num_links-1, OR -1 to mean the base.
	//
	i32 getParent(i32 link_num) const;

	//
	// get number of m_links, masses, moments of inertia
	//

	i32 getNumLinks() const { return m_links.size(); }
	i32 getNumDofs() const { return m_dofCount; }
	i32 getNumPosVars() const { return m_posVarCnt; }
	Scalar getBaseMass() const { return m_baseMass; }
	const Vec3 &getBaseInertia() const { return m_baseInertia; }
	Scalar getLinkMass(i32 i) const;
	const Vec3 &getLinkInertia(i32 i) const;

	//
	// change mass (incomplete: can only change base mass and inertia at present)
	//

	void setBaseMass(Scalar mass) { m_baseMass = mass; }
	void setBaseInertia(const Vec3 &inertia) { m_baseInertia = inertia; }

	//
	// get/set pos/vel/rot/omega for the base link
	//

	const Vec3 &getBasePos() const
	{
		return m_basePos;
	}  // in world frame
	const Vec3 getBaseVel() const
	{
		return Vec3(m_realBuf[3], m_realBuf[4], m_realBuf[5]);
	}  // in world frame
	const Quat &getWorldToBaseRot() const
	{
		return m_baseQuat;
	}

    const Vec3 &getInterpolateBasePos() const
    {
        return m_basePos_interpolate;
    }  // in world frame
    const Quat &getInterpolateWorldToBaseRot() const
    {
        return m_baseQuat_interpolate;
    }

    // rotates world vectors into base frame
	Vec3 getBaseOmega() const { return Vec3(m_realBuf[0], m_realBuf[1], m_realBuf[2]); }  // in world frame

	void setBasePos(const Vec3 &pos)
	{
		m_basePos = pos;
		if(!isBaseKinematic())
			m_basePos_interpolate = pos;
	}

	void setInterpolateBasePos(const Vec3 &pos)
	{
		m_basePos_interpolate = pos;
	}

	void setBaseWorldTransform(const Transform2 &tr)
	{
		setBasePos(tr.getOrigin());
		setWorldToBaseRot(tr.getRotation().inverse());
	}

	Transform2 getBaseWorldTransform() const
	{
		Transform2 tr;
		tr.setOrigin(getBasePos());
		tr.setRotation(getWorldToBaseRot().inverse());
		return tr;
	}

	void setInterpolateBaseWorldTransform(const Transform2 &tr)
	{
		setInterpolateBasePos(tr.getOrigin());
		setInterpolateWorldToBaseRot(tr.getRotation().inverse());
	}

	Transform2 getInterpolateBaseWorldTransform() const
	{
		Transform2 tr;
		tr.setOrigin(getInterpolateBasePos());
		tr.setRotation(getInterpolateWorldToBaseRot().inverse());
		return tr;
	}

	void setBaseVel(const Vec3 &vel)
	{
		m_realBuf[3] = vel[0];
		m_realBuf[4] = vel[1];
		m_realBuf[5] = vel[2];
	}

	void setWorldToBaseRot(const Quat &rot)
	{
		m_baseQuat = rot;  //m_baseQuat asumed to ba alias!?
		if(!isBaseKinematic())
			m_baseQuat_interpolate = rot;
	}

	void setInterpolateWorldToBaseRot(const Quat &rot)
	{
		m_baseQuat_interpolate = rot;
	}

	void setBaseOmega(const Vec3 &omega)
	{
		m_realBuf[0] = omega[0];
		m_realBuf[1] = omega[1];
		m_realBuf[2] = omega[2];
	}

	void saveKinematicState(Scalar timeStep);

	//
	// get/set pos/vel for child m_links (i = 0 to num_links-1)
	//

	Scalar getJointPos(i32 i) const;
	Scalar getJointVel(i32 i) const;

	Scalar *getJointVelMultiDof(i32 i);
	Scalar *getJointPosMultiDof(i32 i);

	const Scalar *getJointVelMultiDof(i32 i) const;
	const Scalar *getJointPosMultiDof(i32 i) const;

	void setJointPos(i32 i, Scalar q);
	void setJointVel(i32 i, Scalar qdot);
	void setJointPosMultiDof(i32 i, const double *q);
	void setJointVelMultiDof(i32 i, const double *qdot);
	void setJointPosMultiDof(i32 i, const float *q);
	void setJointVelMultiDof(i32 i, const float *qdot);

	//
	// direct access to velocities as a vector of 6 + num_links elements.
	// (omega first, then v, then joint velocities.)
	//
	const Scalar *getVelocityVector() const
	{
		return &m_realBuf[0];
	}

    const Scalar *getDeltaVelocityVector() const
    {
        return &m_deltaV[0];
    }

    const Scalar *getSplitVelocityVector() const
    {
        return &m_splitV[0];
    }
	/*    Scalar * getVelocityVector()
	{
		return &real_buf[0];
	}
  */

	//
	// get the frames of reference (positions and orientations) of the child m_links
	// (i = 0 to num_links-1)
	//

	const Vec3 &getRVector(i32 i) const;              // vector from COM(parent(i)) to COM(i), in frame i's coords
	const Quat &getParentToLocalRot(i32 i) const;  // rotates vectors in frame parent(i) to vectors in frame i.
    const Vec3 &getInterpolateRVector(i32 i) const;              // vector from COM(parent(i)) to COM(i), in frame i's coords
    const Quat &getInterpolateParentToLocalRot(i32 i) const;  // rotates vectors in frame parent(i) to vectors in frame i.

	//
	// transform vectors in local frame of link i to world frame (or vice versa)
	//
	Vec3 localPosToWorld(i32 i, const Vec3 &local_pos) const;
	Vec3 localDirToWorld(i32 i, const Vec3 &local_dir) const;
	Vec3 worldPosToLocal(i32 i, const Vec3 &world_pos) const;
	Vec3 worldDirToLocal(i32 i, const Vec3 &world_dir) const;

	//
	// transform a frame in local coordinate to a frame in world coordinate
	//
	Matrix3x3 localFrameToWorld(i32 i, const Matrix3x3 &local_frame) const;


	//
	// set external forces and torques. Note all external forces/torques are given in the WORLD frame.
	//

	void clearForcesAndTorques();
	void clearConstraintForces();

	void clearVelocities();

	void addBaseForce(const Vec3 &f)
	{
		m_baseForce += f;
	}
	void addBaseTorque(const Vec3 &t) { m_baseTorque += t; }
	void addLinkForce(i32 i, const Vec3 &f);
	void addLinkTorque(i32 i, const Vec3 &t);

	void addBaseConstraintForce(const Vec3 &f)
	{
		m_baseConstraintForce += f;
	}
	void addBaseConstraintTorque(const Vec3 &t) { m_baseConstraintTorque += t; }
	void addLinkConstraintForce(i32 i, const Vec3 &f);
	void addLinkConstraintTorque(i32 i, const Vec3 &t);

	void addJointTorque(i32 i, Scalar Q);
	void addJointTorqueMultiDof(i32 i, i32 dof, Scalar Q);
	void addJointTorqueMultiDof(i32 i, const Scalar *Q);

	const Vec3 &getBaseForce() const { return m_baseForce; }
	const Vec3 &getBaseTorque() const { return m_baseTorque; }
	const Vec3 &getLinkForce(i32 i) const;
	const Vec3 &getLinkTorque(i32 i) const;
	Scalar getJointTorque(i32 i) const;
	Scalar *getJointTorqueMultiDof(i32 i);

	//
	// dynamics routines.
	//

	// timestep the velocities (given the external forces/torques set using addBaseForce etc).
	// also sets up caches for calcAccelerationDeltas.
	//
	// Note: the caller must provide three vectors which are used as
	// temporary scratch space. The idea here is to reduce dynamic
	// memory allocation: the same scratch vectors can be re-used
	// again and again for different Multibodies, instead of each
	// MultiBody allocating (and then deallocating) their own
	// individual scratch buffers. This gives a considerable speed
	// improvement, at least on Windows (where dynamic memory
	// allocation appears to be fairly slow).
	//

	void computeAccelerationsArticulatedBodyAlgorithmMultiDof(Scalar dt,
															  AlignedObjectArray<Scalar> & scratch_r,
															  AlignedObjectArray<Vec3> & scratch_v,
															  AlignedObjectArray<Matrix3x3> & scratch_m,
															  bool isConstraintPass,
                                                              bool jointFeedbackInWorldSpace,
                                                              bool jointFeedbackInJointFrame
                                                              );

	///stepVelocitiesMultiDof is deprecated, use computeAccelerationsArticulatedBodyAlgorithmMultiDof instead
	//void stepVelocitiesMultiDof(Scalar dt,
	//							AlignedObjectArray<Scalar> & scratch_r,
	//							AlignedObjectArray<Vec3> & scratch_v,
	//							AlignedObjectArray<Matrix3x3> & scratch_m,
	//							bool isConstraintPass = false)
	//{
	//	computeAccelerationsArticulatedBodyAlgorithmMultiDof(dt, scratch_r, scratch_v, scratch_m, isConstraintPass, false, false);
	//}

	// calcAccelerationDeltasMultiDof
	// input: force vector (in same format as jacobian, i.e.:
	//                      3 torque values, 3 force values, num_links joint torque values)
	// output: 3 omegadot values, 3 vdot values, num_links q_double_dot values
	// (existing contents of output array are replaced)
	// calcAccelerationDeltasMultiDof must have been called first.
	void calcAccelerationDeltasMultiDof(const Scalar *force, Scalar *output,
										AlignedObjectArray<Scalar> &scratch_r,
										AlignedObjectArray<Vec3> &scratch_v) const;

	void applyDeltaVeeMultiDof2(const Scalar *delta_vee, Scalar multiplier)
	{
		for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
		{
			m_deltaV[dof] += delta_vee[dof] * multiplier;
		}
	}
    void applyDeltaSplitVeeMultiDof(const Scalar *delta_vee, Scalar multiplier)
    {
        for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
        {
            m_splitV[dof] += delta_vee[dof] * multiplier;
        }
    }
    void addSplitV()
    {
        applyDeltaVeeMultiDof(&m_splitV[0], 1);
    }
    void substractSplitV()
    {
        applyDeltaVeeMultiDof(&m_splitV[0], -1);

        for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
        {
            m_splitV[dof] = 0.f;
        }
    }
	void processDeltaVeeMultiDof2()
	{
		applyDeltaVeeMultiDof(&m_deltaV[0], 1);

		for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
		{
			m_deltaV[dof] = 0.f;
		}
	}

	void applyDeltaVeeMultiDof(const Scalar *delta_vee, Scalar multiplier)
	{
		//for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
		//	printf("%.4f ", delta_vee[dof]*multiplier);
		//printf("\n");

		//Scalar sum = 0;
		//for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
		//{
		//	sum += delta_vee[dof]*multiplier*delta_vee[dof]*multiplier;
		//}
		//Scalar l = Sqrt(sum);

		//if (l>m_maxAppliedImpulse)
		//{
		//	multiplier *= m_maxAppliedImpulse/l;
		//}

		for (i32 dof = 0; dof < 6 + getNumDofs(); ++dof)
		{
			m_realBuf[dof] += delta_vee[dof] * multiplier;
			Clamp(m_realBuf[dof], -m_maxCoordinateVelocity, m_maxCoordinateVelocity);
		}
	}

	// timestep the positions (given current velocities).
	void stepPositionsMultiDof(Scalar dt, Scalar *pq = 0, Scalar *pqd = 0);

    // predict the positions
    void predictPositionsMultiDof(Scalar dt);

	//
	// contacts
	//

	// This routine fills out a contact constraint jacobian for this body.
	// the 'normal' supplied must be -n for body1 or +n for body2 of the contact.
	// 'normal' & 'contact_point' are both given in world coordinates.

	void fillContactJacobianMultiDof(i32 link,
									 const Vec3 &contact_point,
									 const Vec3 &normal,
									 Scalar *jac,
									 AlignedObjectArray<Scalar> &scratch_r,
									 AlignedObjectArray<Vec3> &scratch_v,
									 AlignedObjectArray<Matrix3x3> &scratch_m) const { fillConstraintJacobianMultiDof(link, contact_point, Vec3(0, 0, 0), normal, jac, scratch_r, scratch_v, scratch_m); }

	//a more general version of fillContactJacobianMultiDof which does not assume..
	//.. that the constraint in question is contact or, to be more precise, constrains linear velocity only
	void fillConstraintJacobianMultiDof(i32 link,
										const Vec3 &contact_point,
										const Vec3 &normal_ang,
										const Vec3 &normal_lin,
										Scalar *jac,
										AlignedObjectArray<Scalar> &scratch_r,
										AlignedObjectArray<Vec3> &scratch_v,
										AlignedObjectArray<Matrix3x3> &scratch_m) const;

	//
	// sleeping
	//
	void setCanSleep(bool canSleep)
	{
		if (m_canWakeup)
		{
			m_canSleep = canSleep;
		}
	}

	bool getCanSleep() const
	{
		return m_canSleep;
	}

	bool getCanWakeup() const
	{
		return m_canWakeup;
	}

	void setCanWakeup(bool canWakeup)
	{
		m_canWakeup = canWakeup;
	}
	bool isAwake() const
	{
		return m_awake;
	}
	void wakeUp();
	void goToSleep();
	void checkMotionAndSleepIfRequired(Scalar timestep);

	bool hasFixedBase() const;

	bool isBaseKinematic() const;

	bool isBaseStaticOrKinematic() const;

	// set the dynamic type in the base's collision flags.
	void setBaseDynamicType(i32 dynamicType);

	void setFixedBase(bool fixedBase)
	{
		m_fixedBase = fixedBase;
		if(m_fixedBase)
			setBaseDynamicType(CollisionObject2::CF_STATIC_OBJECT);
		else
			setBaseDynamicType(CollisionObject2::CF_DYNAMIC_OBJECT);
	}

	i32 getCompanionId() const
	{
		return m_companionId;
	}
	void setCompanionId(i32 id)
	{
		//printf("for %p setCompanionId(%d)\n",this, id);
		m_companionId = id;
	}

	void setNumLinks(i32 numLinks)  //careful: when changing the number of m_links, make sure to re-initialize or update existing m_links
	{
		m_links.resize(numLinks);
	}

	Scalar getLinearDamping() const
	{
		return m_linearDamping;
	}
	void setLinearDamping(Scalar damp)
	{
		m_linearDamping = damp;
	}
	Scalar getAngularDamping() const
	{
		return m_angularDamping;
	}
	void setAngularDamping(Scalar damp)
	{
		m_angularDamping = damp;
	}

	bool getUseGyroTerm() const
	{
		return m_useGyroTerm;
	}
	void setUseGyroTerm(bool useGyro)
	{
		m_useGyroTerm = useGyro;
	}
	Scalar getMaxCoordinateVelocity() const
	{
		return m_maxCoordinateVelocity;
	}
	void setMaxCoordinateVelocity(Scalar maxVel)
	{
		m_maxCoordinateVelocity = maxVel;
	}

	Scalar getMaxAppliedImpulse() const
	{
		return m_maxAppliedImpulse;
	}
	void setMaxAppliedImpulse(Scalar maxImp)
	{
		m_maxAppliedImpulse = maxImp;
	}
	void setHasSelfCollision(bool hasSelfCollision)
	{
		m_hasSelfCollision = hasSelfCollision;
	}
	bool hasSelfCollision() const
	{
		return m_hasSelfCollision;
	}

	void finalizeMultiDof();

	void useRK4Integration(bool use) { m_useRK4 = use; }
	bool isUsingRK4Integration() const { return m_useRK4; }
	void useGlobalVelocities(bool use) { m_useGlobalVelocities = use; }
	bool isUsingGlobalVelocities() const { return m_useGlobalVelocities; }

	bool isPosUpdated() const
	{
		return __posUpdated;
	}
	void setPosUpdated(bool updated)
	{
		__posUpdated = updated;
	}

	//internalNeedsJointFeedback is for internal use only
	bool internalNeedsJointFeedback() const
	{
		return m_internalNeedsJointFeedback;
	}
	void forwardKinematics(AlignedObjectArray<Quat>& world_to_local, AlignedObjectArray<Vec3> & local_origin);

	void compTreeLinkVelocities(Vec3 * omega, Vec3 * vel) const;

	void updateCollisionObjectWorldTransforms(AlignedObjectArray<Quat> & world_to_local, AlignedObjectArray<Vec3> & local_origin);
    void updateCollisionObject2InterpolationWorldTransforms(AlignedObjectArray<Quat> & world_to_local, AlignedObjectArray<Vec3> & local_origin);

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, class Serializer *serializer) const;

	tukk getBaseName() const
	{
		return m_baseName;
	}
	///memory of setBaseName needs to be manager by user
	void setBaseName(tukk name)
	{
		m_baseName = name;
	}

	///users can point to their objects, userPointer is not used by drx3D
	uk getUserPointer() const
	{
		return m_userObjectPointer;
	}

	i32 getUserIndex() const
	{
		return m_userIndex;
	}

	i32 getUserIndex2() const
	{
		return m_userIndex2;
	}
	///users can point to their objects, userPointer is not used by drx3D
	void setUserPointer(uk userPointer)
	{
		m_userObjectPointer = userPointer;
	}

	///users can point to their objects, userPointer is not used by drx3D
	void setUserIndex(i32 index)
	{
		m_userIndex = index;
	}

	void setUserIndex2(i32 index)
	{
		m_userIndex2 = index;
	}

	static void spatialTransform(const Matrix3x3 &rotation_matrix,  // rotates vectors in 'from' frame to vectors in 'to' frame
		const Vec3 &displacement,     // vector from origin of 'from' frame to origin of 'to' frame, in 'to' coordinates
		const Vec3 &top_in,       // top part of input vector
		const Vec3 &bottom_in,    // bottom part of input vector
		Vec3 &top_out,         // top part of output vector
		Vec3 &bottom_out);      // bottom part of output vector

	void setLinkDynamicType(i32k i, i32 type);

	bool isLinkStaticOrKinematic(i32k i) const;

	bool isLinkKinematic(i32k i) const;

	bool isLinkAndAllAncestorsStaticOrKinematic(i32k i) const;

	bool isLinkAndAllAncestorsKinematic(i32k i) const;

	void setSleepThreshold(Scalar sleepThreshold)
	{
		m_sleepEpsilon = sleepThreshold;
	}

	void setSleepTimeout(Scalar sleepTimeout)
	{
		this->m_sleepTimeout = sleepTimeout;
	}


private:
	MultiBody(const MultiBody &);     // not implemented
	void operator=(const MultiBody &);  // not implemented

	void solveImatrix(const Vec3 &rhs_top, const Vec3 &rhs_bot, Scalar result[6]) const;
	void solveImatrix(const SpatialForceVector &rhs, SpatialMotionVector &result) const;

	void updateLinksDofOffsets()
	{
		i32 dofOffset = 0, cfgOffset = 0;
		for (i32 bidx = 0; bidx < m_links.size(); ++bidx)
		{
			m_links[bidx].m_dofOffset = dofOffset;
			m_links[bidx].m_cfgOffset = cfgOffset;
			dofOffset += m_links[bidx].m_dofCount;
			cfgOffset += m_links[bidx].m_posVarCount;
		}
	}

	void mulMatrix(const Scalar *pA, const Scalar *pB, i32 rowsA, i32 colsA, i32 rowsB, i32 colsB, Scalar *pC) const;

private:
	MultiBodyLinkCollider *m_baseCollider;  //can be NULL
	tukk m_baseName;                   //memory needs to be manager by user!

	Vec3 m_basePos;      // position of COM of base (world frame)
    Vec3 m_basePos_interpolate;      // position of interpolated COM of base (world frame)
	Quat m_baseQuat;  // rotates world points into base frame
    Quat m_baseQuat_interpolate;

	Scalar m_baseMass;      // mass of the base
	Vec3 m_baseInertia;  // inertia of the base (in local frame; diagonal)

	Vec3 m_baseForce;   // external force applied to base. World frame.
	Vec3 m_baseTorque;  // external torque applied to base. World frame.

	Vec3 m_baseConstraintForce;   // external force applied to base. World frame.
	Vec3 m_baseConstraintTorque;  // external torque applied to base. World frame.

	AlignedObjectArray<MultibodyLink> m_links;  // array of m_links, excluding the base. index from 0 to num_links-1.

	//
	// realBuf:
	//  offset         size            array
	//   0              6 + num_links   v (base_omega; base_vel; joint_vels)					MULTIDOF [sysdof x sysdof for D matrices (TOO MUCH!) + pos_delta which is sys-cfg sized]
	//   6+num_links    num_links       D
	//
	// vectorBuf:
	//  offset         size         array
	//   0              num_links    h_top
	//   num_links      num_links    h_bottom
	//
	// matrixBuf:
	//  offset         size         array
	//   0              num_links+1  rot_from_parent
	//
    AlignedObjectArray<Scalar> m_splitV;
	AlignedObjectArray<Scalar> m_deltaV;
	AlignedObjectArray<Scalar> m_realBuf;
	AlignedObjectArray<Vec3> m_vectorBuf;
	AlignedObjectArray<Matrix3x3> m_matrixBuf;

	Matrix3x3 m_cachedInertiaTopLeft;
	Matrix3x3 m_cachedInertiaTopRight;
	Matrix3x3 m_cachedInertiaLowerLeft;
	Matrix3x3 m_cachedInertiaLowerRight;
	bool m_cachedInertiaValid;

	bool m_fixedBase;

	// Sleep parameters.
	bool m_awake;
	bool m_canSleep;
	bool m_canWakeup;
	Scalar m_sleepTimer;
	Scalar m_sleepEpsilon;
	Scalar m_sleepTimeout;

	uk m_userObjectPointer;
	i32 m_userIndex2;
	i32 m_userIndex;

	i32 m_companionId;
	Scalar m_linearDamping;
	Scalar m_angularDamping;
	bool m_useGyroTerm;
	Scalar m_maxAppliedImpulse;
	Scalar m_maxCoordinateVelocity;
	bool m_hasSelfCollision;

	bool __posUpdated;
	i32 m_dofCount, m_posVarCnt;

	bool m_useRK4, m_useGlobalVelocities;
	//for global velocities, see 8.3.2B Proposed resolution in Jakub Stepien PhD Thesis
	//https://drive.google.com/file/d/0Bz3vEa19XOYGNWdZWGpMdUdqVmZ5ZVBOaEh4ZnpNaUxxZFNV/view?usp=sharing

	///the m_needsJointFeedback gets updated/computed during the stepVelocitiesMultiDof and it for internal usage only
	bool m_internalNeedsJointFeedback;

  //If enabled, calculate the velocity based on kinematic transform changes. Currently only implemented for the base.
	bool m_kinematic_calculate_velocity;
};

struct MultiBodyLinkDoubleData
{
	QuatDoubleData m_zeroRotParentToThis;
	Vec3DoubleData m_parentComToThisPivotOffset;
	Vec3DoubleData m_thisPivotToThisComOffset;
	Vec3DoubleData m_jointAxisTop[6];
	Vec3DoubleData m_jointAxisBottom[6];

	Vec3DoubleData m_linkInertia;  // inertia of the base (in local frame; diagonal)
	Vec3DoubleData m_absFrameTotVelocityTop;
	Vec3DoubleData m_absFrameTotVelocityBottom;
	Vec3DoubleData m_absFrameLocVelocityTop;
	Vec3DoubleData m_absFrameLocVelocityBottom;

	double m_linkMass;
	i32 m_parentIndex;
	i32 m_jointType;

	i32 m_dofCount;
	i32 m_posVarCount;
	double m_jointPos[7];
	double m_jointVel[6];
	double m_jointTorque[6];

	double m_jointDamping;
	double m_jointFriction;
	double m_jointLowerLimit;
	double m_jointUpperLimit;
	double m_jointMaxForce;
	double m_jointMaxVelocity;

	char *m_linkName;
	char *m_jointName;
	CollisionObject2DoubleData *m_linkCollider;
	char *m_paddingPtr;
};

struct MultiBodyLinkFloatData
{
	QuatFloatData m_zeroRotParentToThis;
	Vec3FloatData m_parentComToThisPivotOffset;
	Vec3FloatData m_thisPivotToThisComOffset;
	Vec3FloatData m_jointAxisTop[6];
	Vec3FloatData m_jointAxisBottom[6];
	Vec3FloatData m_linkInertia;  // inertia of the base (in local frame; diagonal)
	Vec3FloatData m_absFrameTotVelocityTop;
	Vec3FloatData m_absFrameTotVelocityBottom;
	Vec3FloatData m_absFrameLocVelocityTop;
	Vec3FloatData m_absFrameLocVelocityBottom;

	i32 m_dofCount;
	float m_linkMass;
	i32 m_parentIndex;
	i32 m_jointType;

	float m_jointPos[7];
	float m_jointVel[6];
	float m_jointTorque[6];
	i32 m_posVarCount;
	float m_jointDamping;
	float m_jointFriction;
	float m_jointLowerLimit;
	float m_jointUpperLimit;
	float m_jointMaxForce;
	float m_jointMaxVelocity;

	char *m_linkName;
	char *m_jointName;
	CollisionObject2FloatData *m_linkCollider;
	char *m_paddingPtr;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct MultiBodyDoubleData
{
	Vec3DoubleData m_baseWorldPosition;
	QuatDoubleData m_baseWorldOrientation;
	Vec3DoubleData m_baseLinearVelocity;
	Vec3DoubleData m_baseAngularVelocity;
	Vec3DoubleData m_baseInertia;  // inertia of the base (in local frame; diagonal)
	double m_baseMass;
	i32 m_numLinks;
	char m_padding[4];

	char *m_baseName;
	MultiBodyLinkDoubleData *m_links;
	CollisionObject2DoubleData *m_baseCollider;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct MultiBodyFloatData
{
	Vec3FloatData m_baseWorldPosition;
	QuatFloatData m_baseWorldOrientation;
	Vec3FloatData m_baseLinearVelocity;
	Vec3FloatData m_baseAngularVelocity;

	Vec3FloatData m_baseInertia;  // inertia of the base (in local frame; diagonal)
	float m_baseMass;
	i32 m_numLinks;

	char *m_baseName;
	MultiBodyLinkFloatData *m_links;
	CollisionObject2FloatData *m_baseCollider;
};

#endif
