#ifndef MULTIBODYTREE_H_
#define MULTIBODYTREE_H_

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/IDMath.h>

namespace drx3d_inverse
{
/// Enumeration of supported joint types
enum JointType
{
	/// no degree of freedom, moves with parent
	FIXED = 0,
	/// one rotational degree of freedom relative to parent
	REVOLUTE,
	/// one translational degree of freedom relative to parent
	PRISMATIC,
	/// six degrees of freedom relative to parent
	FLOATING,
	/// three degrees of freedom, relative to parent
	SPHERICAL
};

/// Interface class for calculating inverse dynamics for tree structured
/// multibody systems
///
/// Note on degrees of freedom
/// The q vector contains the generalized coordinate set defining the tree's configuration.
/// Every joint adds elements that define the corresponding link's frame pose relative to
/// its parent. For the joint types that is:
///	- FIXED:	 none
///	- REVOLUTE:  angle of rotation [rad]
///	- PRISMATIC: displacement [m]
///	- FLOATING:  Euler x-y-z angles [rad] and displacement in body-fixed frame of parent [m]
///				 (in that order)
/// - SPHERICAL: Euler x-y-z angles [rad]
/// The u vector contains the generalized speeds, which are
///	- FIXED:	 none
///	- REVOLUTE:  time derivative of angle of rotation [rad/s]
///	- PRISMATIC: time derivative of displacement [m/s]
///	- FLOATING:  angular velocity [rad/s] (*not* time derivative of rpy angles)
///				 and time derivative of displacement in parent frame [m/s]
//	- SPHERICAL:  angular velocity [rad/s]
///
/// The q and u vectors are obtained by stacking contributions of all bodies in one
/// vector in the order of body indices.
///
/// Note on generalized forces: analogous to u, i.e.,
///	 - FIXED:	 none
///	 - REVOLUTE:  moment [Nm], about joint axis
///	 - PRISMATIC: force  [N], along joint axis
///	 - FLOATING:  moment vector [Nm] and force vector [N], both in body-fixed frame
///				  (in that order)
///  - SPHERICAL: moment vector [Nm] 
/// TODO - force element interface (friction, springs, dampers, etc)
///	  - gears and motor inertia
class MultiBodyTree
{
public:
	ID_DECLARE_ALIGNED_ALLOCATOR();
	/// The contructor.
	/// Initialization & allocation is via addBody and buildSystem calls.
	MultiBodyTree();
	/// the destructor. This also deallocates all memory
	~MultiBodyTree();

	/// Add body to the system. this allocates memory and not real-time safe.
	/// This only adds the data to an initial cache. After all bodies have been
	/// added,
	/// the system is setup using the buildSystem call
	/// @param body_index index of the body to be added. Must >=0, <number of bodies,
	///		and index of parent must be < index of body
	/// @param parent_index index of the parent body
	///		The root of the tree has index 0 and its parent (the world frame)
	///		is assigned index -1
	///		the rotation and translation relative to the parent are taken as
	///		pose of the root body relative to the world frame. Other parameters
	///		are ignored
	/// @param JointType type of joint connecting the body to the parent
	/// @param mass the mass of the body
	/// @param body_r_body_com the center of mass of the body relative to and
	/// described in
	///		the body fixed frame, which is located in the joint axis connecting
	/// the body to its parent
	/// @param body_I_body the moment of inertia of the body w.r.t the body-fixed
	/// frame
	///		(ie, the reference point is the origin of the body-fixed frame and
	/// the matrix is written
	///		 w.r.t. those unit vectors)
	/// @param parent_r_parent_body_ref position of joint relative to the parent
	/// body's reference frame
	///		for q=0, written in the parent bodies reference frame
	/// @param body_axis_of_motion translation/rotation axis in body-fixed frame.
	///		Ignored for joints that are not revolute or prismatic.
	///		must be a unit vector.
	/// @param body_T_parent_ref transform matrix from parent to body reference
	/// frame for q=0.
	///		This is the matrix transforming a vector represented in the
	/// parent's reference frame into one represented
	///		in this body's reference frame.
	///		ie, if parent_vec is a vector in R^3 whose components are w.r.t to
	/// the parent's reference frame,
	///		then the same vector written w.r.t. this body's frame (for q=0) is
	/// given by
	///		body_vec = parent_R_body_ref * parent_vec
	/// @param user_ptr pointer to user data
	/// @param user_int pointer to user integer
	/// @return 0 on success, -1 on error
	i32 addBody(i32 body_index, i32 parent_index, JointType joint_type,
				const vec3& parent_r_parent_body_ref, const mat33& body_T_parent_ref,
				const vec3& body_axis_of_motion, idScalar mass, const vec3& body_r_body_com,
				const mat33& body_I_body, i32k user_int, uk user_ptr);
	/// set policy for invalid mass properties
	/// @param flag if true, invalid mass properties are accepted,
	///		the default is false
	void setAcceptInvalidMassParameters(bool flag);
	/// @return the mass properties policy flag
	bool getAcceptInvalidMassProperties() const;
	/// build internal data structures
	/// call this after all bodies have been added via addBody
	/// @return 0 on success, -1 on error
	i32 finalize();
	/// pretty print ascii description of tree to stdout
	void printTree();
	/// print tree data to stdout
	void printTreeData();
	/// Calculate joint forces for given generalized state & derivatives.
	/// This also updates kinematic terms computed in calculateKinematics.
	/// If gravity is not set to zero, acceleration terms will contain
	/// gravitational acceleration.
	/// @param q generalized coordinates
	/// @param u generalized velocities. In the general case, u=T(q)*dot(q) and dim(q)>=dim(u)
	/// @param dot_u time derivative of u
	/// @param joint_forces this is where the resulting joint forces will be
	///		stored. dim(joint_forces) = dim(u)
	/// @return 0 on success, -1 on error
	i32 calculatedrx3d_inverse(const vecx& q, const vecx& u, const vecx& dot_u,
								 vecx* joint_forces);
	/// Calculate joint space mass matrix
	/// @param q generalized coordinates
	/// @param initialize_matrix if true, initialize mass matrix with zero.
	///		If mass_matrix is initialized to zero externally and only used
	///		for mass matrix computations for the same system, it is safe to
	///		set this to false.
	/// @param set_lower_triangular_matrix if true, the lower triangular section of mass_matrix
	///		is also populated, otherwise not.
	/// @param mass_matrix matrix for storing the output (should be dim(q)xdim(q))
	/// @return -1 on error, 0 on success
	i32 calculateMassMatrix(const vecx& q, const bool update_kinematics,
							const bool initialize_matrix, const bool set_lower_triangular_matrix,
							matxx* mass_matrix);

	/// Calculate joint space mass matrix.
	/// This version will update kinematics, initialize all mass_matrix elements to zero and
	/// populate all mass matrix entries.
	/// @param q generalized coordinates
	/// @param mass_matrix matrix for storing the output (should be dim(q)xdim(q))
	/// @return -1 on error, 0 on success
	i32 calculateMassMatrix(const vecx& q, matxx* mass_matrix);

	/// Calculates kinematics also calculated in calculatedrx3d_inverse,
	/// but not dynamics.
	/// This function ensures that correct accelerations are computed that do not
	/// contain gravitational acceleration terms.
	/// Does not calculate Jacobians, but only vector quantities (positions, velocities & accelerations)
	i32 calculateKinematics(const vecx& q, const vecx& u, const vecx& dot_u);
	/// Calculate position kinematics
	i32 calculatePositionKinematics(const vecx& q);
	/// Calculate position and velocity kinematics
	i32 calculatePositionAndVelocityKinematics(const vecx& q, const vecx& u);

#if (defined DRX3D_ID_HAVE_MAT3X) && (defined DRX3D_ID_WITH_JACOBIANS)
	/// Calculate Jacobians (dvel/du), as well as velocity-dependent accelearation components
	/// d(Jacobian)/dt*u
	/// This function assumes that calculatedrx3d_inverse was called, or calculateKinematics,
	/// or calculatePositionAndVelocityKinematics
	i32 calculateJacobians(const vecx& q, const vecx& u);
	/// Calculate Jacobians (dvel/du)
	/// This function assumes that calculatedrx3d_inverse was called, or
	/// one of the calculateKineamtics functions
	i32 calculateJacobians(const vecx& q);
#endif  // DRX3D_ID_HAVE_MAT3X

	/// set gravitational acceleration
	/// the default is [0;0;-9.8] in the world frame
	/// @param gravity the gravitational acceleration in world frame
	/// @return 0 on success, -1 on error
	i32 setGravityInWorldFrame(const vec3& gravity);
	/// returns number of bodies in tree
	i32 numBodies() const;
	/// returns number of mechanical degrees of freedom (dimension of q-vector)
	i32 numDoFs() const;
	/// get origin of a body-fixed frame, represented in world frame
	/// @param body_index index for frame/body
	/// @param world_origin pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyOrigin(i32k body_index, vec3* world_origin) const;
	/// get center of mass of a body, represented in world frame
	/// @param body_index index for frame/body
	/// @param world_com pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyCoM(i32k body_index, vec3* world_com) const;
	/// get transform from of a body-fixed frame to the world frame
	/// @param body_index index for frame/body
	/// @param world_T_body pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyTransform(i32k body_index, mat33* world_T_body) const;
	/// get absolute angular velocity for a body, represented in the world frame
	/// @param body_index index for frame/body
	/// @param world_omega pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyAngularVelocity(i32k body_index, vec3* world_omega) const;
	/// get linear velocity of a body, represented in world frame
	/// @param body_index index for frame/body
	/// @param world_velocity pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyLinearVelocity(i32k body_index, vec3* world_velocity) const;
	/// get linear velocity of a body's CoM, represented in world frame
	/// (not required for inverse dynamics, provided for convenience)
	/// @param body_index index for frame/body
	/// @param world_vel_com pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyLinearVelocityCoM(i32k body_index, vec3* world_velocity) const;
	/// get origin of a body-fixed frame, represented in world frame
	/// @param body_index index for frame/body
	/// @param world_origin pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyAngularAcceleration(i32k body_index, vec3* world_dot_omega) const;
	/// get origin of a body-fixed frame, represented in world frame
	/// NOTE: this will include the gravitational acceleration, so the actual acceleration is
	/// obtainened by setting gravitational acceleration to zero, or subtracting it.
	/// @param body_index index for frame/body
	/// @param world_origin pointer for return data
	/// @return 0 on success, -1 on error
	i32 getBodyLinearAcceleration(i32k body_index, vec3* world_acceleration) const;

#if (defined DRX3D_ID_HAVE_MAT3X) && (defined DRX3D_ID_WITH_JACOBIANS)
	// get translational jacobian, in world frame (dworld_velocity/du)
	i32 getBodyJacobianTrans(i32k body_index, mat3x* world_jac_trans) const;
	// get rotational jacobian, in world frame (dworld_omega/du)
	i32 getBodyJacobianRot(i32k body_index, mat3x* world_jac_rot) const;
	// get product of translational jacobian derivative * generatlized velocities
	i32 getBodyDotJacobianTransU(i32k body_index, vec3* world_dot_jac_trans_u) const;
	// get product of rotational jacobian derivative * generatlized velocities
	i32 getBodyDotJacobianRotU(i32k body_index, vec3* world_dot_jac_rot_u) const;
#endif  // DRX3D_ID_HAVE_MAT3X

	/// returns the (internal) index of body
	/// @param body_index is the index of a body
	/// @param parent_index pointer to where parent index will be stored
	/// @return 0 on success, -1 on error
	i32 getParentIndex(i32k body_index, i32* parent_index) const;
	/// get joint type
	/// @param body_index index of the body
	/// @param joint_type the corresponding joint type
	/// @return 0 on success, -1 on failure
	i32 getJointType(i32k body_index, JointType* joint_type) const;
	/// get joint type as string
	/// @param body_index index of the body
	/// @param joint_type string naming the corresponding joint type
	/// @return 0 on success, -1 on failure
	i32 getJointTypeStr(i32k body_index, tukk* joint_type) const;
	/// get offset translation to parent body (see addBody)
	/// @param body_index index of the body
	/// @param r the offset translation (see above)
	/// @return 0 on success, -1 on failure
	i32 getParentRParentBodyRef(i32k body_index, vec3* r) const;
	/// get offset rotation to parent body (see addBody)
	/// @param body_index index of the body
	/// @param T the transform (see above)
	/// @return 0 on success, -1 on failure
	i32 getBodyTParentRef(i32k body_index, mat33* T) const;
	/// get axis of motion (see addBody)
	/// @param body_index index of the body
	/// @param axis the axis (see above)
	/// @return 0 on success, -1 on failure
	i32 getBodyAxisOfMotion(i32k body_index, vec3* axis) const;
	/// get offset for degrees of freedom of this body into the q-vector
	/// @param body_index index of the body
	/// @param q_offset offset the q vector
	/// @return -1 on error, 0 on success
	i32 getDoFOffset(i32k body_index, i32* q_offset) const;
	/// get user integer. not used by the library.
	/// @param body_index index of the body
	/// @param user_int   the user integer
	/// @return 0 on success, -1 on error
	i32 getUserInt(i32k body_index, i32* user_int) const;
	/// get user pointer. not used by the library.
	/// @param body_index index of the body
	/// @param user_ptr   the user pointer
	/// @return 0 on success, -1 on error
	i32 getUserPtr(i32k body_index, uk * user_ptr) const;
	/// set user integer. not used by the library.
	/// @param body_index index of the body
	/// @param user_int   the user integer
	/// @return 0 on success, -1 on error
	i32 setUserInt(i32k body_index, i32k user_int);
	/// set user pointer. not used by the library.
	/// @param body_index index of the body
	/// @param user_ptr   the user pointer
	/// @return 0 on success, -1 on error
	i32 setUserPtr(i32k body_index, uk const user_ptr);
	/// set mass for a body
	/// @param body_index index of the body
	/// @param mass the mass to set
	/// @return 0 on success, -1 on failure
	i32 setBodyMass(i32k body_index, const idScalar mass);
	/// set first moment of mass for a body
	/// (mass * center of mass, in body fixed frame, relative to joint)
	/// @param body_index index of the body
	/// @param first_mass_moment the vector to set
	/// @return 0 on success, -1 on failure
	i32 setBodyFirstMassMoment(i32k body_index, const vec3& first_mass_moment);
	/// set second moment of mass for a body
	/// (moment of inertia, in body fixed frame, relative to joint)
	/// @param body_index index of the body
	/// @param second_mass_moment the inertia matrix
	/// @return 0 on success, -1 on failure
	i32 setBodySecondMassMoment(i32k body_index, const mat33& second_mass_moment);
	/// get mass for a body
	/// @param body_index index of the body
	/// @param mass the mass
	/// @return 0 on success, -1 on failure
	i32 getBodyMass(i32k body_index, idScalar* mass) const;
	/// get first moment of mass for a body
	/// (mass * center of mass, in body fixed frame, relative to joint)
	/// @param body_index index of the body
	/// @param first_moment the vector
	/// @return 0 on success, -1 on failure
	i32 getBodyFirstMassMoment(i32k body_index, vec3* first_mass_moment) const;
	/// get second moment of mass for a body
	/// (moment of inertia, in body fixed frame, relative to joint)
	/// @param body_index index of the body
	/// @param second_mass_moment the inertia matrix
	/// @return 0 on success, -1 on failure
	i32 getBodySecondMassMoment(i32k body_index, mat33* second_mass_moment) const;
	/// set all user forces and moments to zero
	void clearAllUserForcesAndMoments();
	/// Add an external force to a body, acting at the origin of the body-fixed frame.
	/// Calls to addUserForce are cumulative. Set the user force and moment to zero
	/// via clearAllUserForcesAndMoments()
	/// @param body_force the force represented in the body-fixed frame of reference
	/// @return 0 on success, -1 on error
	i32 addUserForce(i32k body_index, const vec3& body_force);
	/// Add an external moment to a body.
	/// Calls to addUserMoment are cumulative. Set the user force and moment to zero
	/// via clearAllUserForcesAndMoments()
	/// @param body_moment the moment represented in the body-fixed frame of reference
	/// @return 0 on success, -1 on error
	i32 addUserMoment(i32k body_index, const vec3& body_moment);

private:
	// flag indicating if system has been initialized
	bool m_is_finalized;
	// flag indicating if mass properties are physically valid
	bool m_mass_parameters_are_valid;
	// flag defining if unphysical mass parameters are accepted
	bool m_accept_invalid_mass_parameters;
	// This struct implements the inverse dynamics calculations
	class MultiBodyImpl;
	MultiBodyImpl* m_impl;
	// cache data structure for initialization
	class InitCache;
	InitCache* m_init_cache;
};
}  // namespace drx3d_inverse
#endif  // MULTIBODYTREE_H_
