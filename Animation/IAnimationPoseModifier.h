// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/IDrxUnknown.h>

//

struct ISkeletonPose;
struct ISkeletonAnim;

struct IAnimationPoseData;
struct IAnimationPoseModifier;

//

struct SAnimationPoseModifierParams
{
	ICharacterInstance*           pCharacterInstance;
	IAnimationPoseData*           pPoseData;

	f32                           timeDelta;

	QuatTS                        location;

	ILINE ISkeletonPose*          GetISkeletonPose() const    { return pCharacterInstance->GetISkeletonPose(); }
	ILINE ISkeletonAnim*          GetISkeletonAnim() const    { return pCharacterInstance->GetISkeletonAnim(); }
	ILINE const IDefaultSkeleton& GetIDefaultSkeleton() const { return pCharacterInstance->GetIDefaultSkeleton(); }
};

//

struct IAnimationPoseData
{
	// <interfuscator:shuffle>

	virtual ~IAnimationPoseData() {}

	/**
	 * Retrieves the total number of joints stored within this pose.
	 * @return Number of joints.
	 */
	virtual u32 GetJointCount() const = 0;

	/**
	 * Retrieves parent-space location of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current location value.
	 */
	virtual const QuatT& GetJointRelative(u32k index) const = 0;

	/**
	 * Retrieves parent-space position of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current position value.
	 */
	virtual const Vec3& GetJointRelativeP(u32k index) const = 0;

	/**
	 * Retrieves parent-space orientation of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current orientation value.
	 */
	virtual const Quat& GetJointRelativeO(u32k index) const = 0;

	/**
	 * Retrieves parent-space scaling of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current scaling value.
	 */
	virtual const Diag33 GetJointRelativeS(u32k index) const = 0;

	/**
	 * Sets parent-space location for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New location value.
	 */
	virtual void SetJointRelative(u32k index, const QuatT& transformation) = 0;

	/**
	 * Sets parent-space position for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New position value.s
	 */
	virtual void SetJointRelativeP(u32k index, const Vec3& position) = 0;

	/**
	 * Sets parent-space orientation for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New orientation value.
	 */
	virtual void SetJointRelativeO(u32k index, const Quat& orientation) = 0;

	/**
	 * Sets parent-space uniform scaling for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New scaling value.
	 */
	virtual void SetJointRelativeS(u32k index, const float scaling) = 0;

	/**
	 * Retrieves model-space location of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current location value.
	 */
	virtual const QuatT& GetJointAbsolute(u32k index) const = 0;

	/**
	 * Retrieves model-space position of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current position value.
	 */
	virtual const Vec3& GetJointAbsoluteP(u32k index) const = 0;

	/**
	 * Retrieves model-space orientation of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current orientation value.
	 */
	virtual const Quat& GetJointAbsoluteO(u32k index) const = 0;

	/**
	 * Retrieves model-space scaling of a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @return Current scaling value.
	 */
	virtual const Diag33 GetJointAbsoluteS(u32k index) const = 0;

	/**
	 * Sets model-space location for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New location value.
	 */
	virtual void SetJointAbsolute(u32k index, const QuatT& transformation) = 0;

	/**
	 * Sets model-space position for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New position value.
	 */
	virtual void SetJointAbsoluteP(u32k index, const Vec3& position) = 0;

	/**
	 * Sets model-space orientation for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New orientation value.
	 */
	virtual void SetJointAbsoluteO(u32k index, const Quat& orientation) = 0;

	/**
	 * Sets model-space uniform scaling for a single joint within this pose.
	 * @param index Index of the joint. Must be lower than the number of joints.
	 * @param transformation New scaling value.
	 */
	virtual void SetJointAbsoluteS(u32k index, const float scaling) = 0;

	// </interfuscator:shuffle>
};

//

//! Interface used for modifying the animated pose of a character, for example for Inverse Kinematics 
struct IAnimationPoseModifier :
	public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseModifier, "22fe4775-5e42-447f-bab6-274ed39af449"_drx_guid);

	// <interfuscator:shuffle>
	//! Command Buffer. Pose data will not be available at this stage.
	// Called from the main thread before the Pose Modifier is added to the.
	virtual bool Prepare(const SAnimationPoseModifierParams& params) = 0;

	//! Called from an arbitrary worker thread when the Command associated with
	//! this Pose Modifier is executed. Pose data is available for read/write.
	virtual bool Execute(const SAnimationPoseModifierParams& params) = 0;

	//! Called from the main thread after the Command Buffer this Pose Modifier
	//! was part of finished its execution.
	virtual void Synchronize() = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseModifier);

//

struct IAnimationPoseModifierSetup :
	public IAnimationSerializable
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseModifierSetup, "59b4f3ae-6197-4bee-ba60-d361b7975e69"_drx_guid)

	// <interfuscator:shuffle>
	virtual IAnimationPoseModifier* GetEntry(i32 index) = 0;
	virtual i32 GetEntryCount() = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseModifierSetup);

//

//! Animation pose modifier that allows for overriding the orientation of individual joints
//! \par Example
//! \include DinrusXAnimation/Examples/OperatorQueue.cpp
struct IAnimationOperatorQueue :
	public IAnimationPoseModifier
{
	DRXINTERFACE_DECLARE_GUID(IAnimationOperatorQueue, "686a56d5-215d-44dd-a166-ccf13327d8a2"_drx_guid);

	enum EOp
	{
		eOp_Override,
		eOp_OverrideRelative,
		eOp_OverrideWorld,
		eOp_Additive,
		eOp_AdditiveRelative,
	};

	// <interfuscator:shuffle>
	virtual void PushPosition(u32 jointIndex, EOp eOp, const Vec3& value) = 0;
	virtual void PushOrientation(u32 jointIndex, EOp eOp, const Quat& value) = 0;

	virtual void PushStoreRelative(u32 jointIndex, QuatT& output) = 0;
	virtual void PushStoreAbsolute(u32 jointIndex, QuatT& output) = 0;
	virtual void PushStoreWorld(u32 jointIndex, QuatT& output) = 0;

	virtual void PushComputeAbsolute() = 0;

	virtual void Clear() = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationOperatorQueue);

// Pose modifier that allows for directing a joint chain towards a target location
//! \par Example (Look-IK)
//! \include DinrusXAnimation/Examples/LookIK.cpp
//! \par Example (Aim-IK)
//! \include DinrusXAnimation/Examples/AimIK.cpp
struct IAnimationPoseBlenderDir :
	public IAnimationPoseModifier
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseBlenderDir, "1725a49d-bd68-4ff4-852c-d0d4b7f86c28"_drx_guid);

	// <interfuscator:shuffle>
	virtual void SetState(bool state) = 0;
	//! Location of the target we want to aim at, in world coordinates
	virtual void SetTarget(const Vec3& target) = 0;
	virtual void SetLayer(u32 nLayer) = 0;
	virtual void SetFadeoutAngle(f32 angleRadians) = 0;
	virtual void SetFadeOutSpeed(f32 time) = 0;
	virtual void SetFadeInSpeed(f32 time) = 0;
	virtual void SetFadeOutMinDistance(f32 minDistance) = 0;
	virtual void SetPolarCoordinatesOffset(const Vec2& offset) = 0;
	virtual void SetPolarCoordinatesSmoothTimeSeconds(f32 smoothTimeSeconds) = 0;
	virtual void SetPolarCoordinatesMaxRadiansPerSecond(const Vec2& maxRadiansPerSecond) = 0;
	virtual f32  GetBlend() const = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseBlenderDir);

//

struct IAnimationGroundAlignment :
	public IAnimationPoseModifier
{
	DRXINTERFACE_DECLARE_GUID(IAnimationGroundAlignment, "b8bf63b9-8d30-4d7b-aaa5-fbdf665715b2"_drx_guid);

	virtual void SetData(const bool bAlignSkeletonVertical, const f32 rootHeight, const Plane& planeLeft, const Plane& planeRight) = 0;
};

DECLARE_SHARED_POINTERS(IAnimationGroundAlignment);

//

//! Used to process more complex pose modifiers such as feet ground alignment.
struct IAnimationPoseAlignerChain :
	public IAnimationPoseModifier
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseAlignerChain, "f5d18a45-8249-45b5-9f68-e45aa9687c4a"_drx_guid);

	enum EType
	{
		eType_Limb
	};

	enum ELockMode
	{
		eLockMode_Off,
		eLockMode_Store,
		eLockMode_Apply,
	};

	struct STarget
	{
		Plane plane;
		float distance;
		float offsetMin;
		float offsetMax;
		float targetWeight;
		float alignWeight;
	};

	// <interfuscator:shuffle>
	virtual void Initialize(LimbIKDefinitionHandle solver, i32 contactJointIndex) = 0;

	virtual void SetTarget(const STarget& target) = 0;
	virtual void SetTargetLock(ELockMode eLockMode) = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseAlignerChain);

//

//! Used to support blending from one state (such as ragdoll) to an animated pose
struct IAnimationPoseMatching :
	public IAnimationPoseModifier
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseMatching, "a988bda5-5940-4438-b69a-1f57e1301815"_drx_guid);

	// <interfuscator:shuffle>
	virtual void SetAnimations(u32k* pAnimationIds, u32 count) = 0;
	virtual bool GetMatchingAnimation(u32& animationId) const = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseMatching);

//

struct IAnimationPoseAligner :
	public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IAnimationPoseAligner, "5c852e72-6d44-7cb0-9f7f-5c80c41b429a"_drx_guid);

	// <interfuscator:shuffle>
	virtual bool Initialize(IEntity& entity, ICharacterInstance* pCharacter) = 0;
	virtual void Clear() = 0;

	virtual void SetRootOffsetEnable(bool bEnable) = 0;
	virtual void SetBlendWeight(float weight) = 0;

	virtual void Update(ICharacterInstance* pCharacter, const QuatT& location, const float time) = 0;
	// </interfuscator:shuffle>
};

DECLARE_SHARED_POINTERS(IAnimationPoseAligner);