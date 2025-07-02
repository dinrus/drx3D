// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
Описание: 

-------------------------------------------------------------------------
История:
- 23:2:2010	17:14 : Created by David Ramos
*************************************************************************/
#pragma once
#ifndef __HIT_DEATH_REACTIONS_DEFS_H
#define __HIT_DEATH_REACTIONS_DEFS_H

#include <drx3D/Script/ScriptHelpers.h>							// SmartScriptTable
#include <drx3D/AI/IAgent.h>											// EStance enumeration
#include <drx3D/Animation/DrxCharAnimationParams.h>			// anim flags
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IDrxMannequin.h>


//	Utility enum for specifying cardinal directions
enum ECardinalDirection
{
	eCD_Invalid = -1,

	// "90�" directions
	eCD_Forward,
	eCD_Back,
	eCD_Left,
	eCD_Right,

	// "180�" direction
	eCD_Ahead,
	eCD_Behind,
	eCD_LeftSide,
	eCD_RightSide,
};

enum EAirState
{
	eAS_Unset = -1,
	eAS_OnGround,
	eAS_OnObject,
	eAS_OnGroundOrObject,
	eAS_InAir,
};

// For the streaming request and releases
enum EReactionsRequestsFlags
{
	eRRF_Alive				= 1,
	eRRF_OutFromPool	= 2,
	eRRF_AIEnabled		= 4,
};

// Utility typedefs
typedef SmartScriptTable	ScriptTablePtr;
enum { INVALID_PROFILE_ID = 0xFFFFFFFF };
typedef u32						ProfileId;
typedef i32								ReactionId;

// Forward declarations
class CHitDeathReactions;

// 
namespace
{
	const char VALIDATION_ID[] = "__validationId";
	const char REACTION_ID[] = "__reactionId";
	const ReactionId INVALID_REACTION_ID = 0;
	u8k NO_COLLISION_REACTION = 0;
	u32k DEFAULT_REACTION_ANIM_FLAGS = CA_FORCE_SKELETON_UPDATE | CA_DISABLE_MULTILAYER | CA_REPEAT_LAST_KEY | CA_ALLOW_ANIM_RESTART | CA_FORCE_TRANSITION_TO_ANIM;
	const char HIT_DEATH_REACTIONS_SCRIPT_TABLE[] = "HitDeathReactions";
}

//////////////////////////////////////////////////////////////////////////
// Functor for random_shuffle
//////////////////////////////////////////////////////////////////////////
struct SRandomGeneratorFunct
{
	SRandomGeneratorFunct(CMTRand_int32& pseudoRandomGenerator);

	template <typename Distance>
	Distance operator () (const Distance& n)
	{
		return static_cast<Distance>(m_pseudoRandomGenerator.GenerateUint32() % n);
	}

	CMTRand_int32& m_pseudoRandomGenerator;
};

class CFragmentCache;

//////////////////////////////////////////////////////////////////////////
// Struct holding all the reaction parameters
//////////////////////////////////////////////////////////////////////////
struct SReactionParams
{
	typedef std::vector<u32> AnimCRCContainer;

	//////////////////////////////////////////////////////////////////////////
	struct SReactionAnim
	{
		SReactionAnim();
		~SReactionAnim();

		void Reset();

		i32	 GetNextReactionAnimIndex() const;
		i32	 GetNextReactionAnimId(const IAnimationSet* pAnimSet) const;
		void RequestNextAnim(const IAnimationSet* pAnimSet) const;
		void ReleaseRequestedAnims();

		i32												iLayer;
		u32										animFlags;
		float											fOverrideTransTimeToAG; // Used for overriding the transition time the animation on the current AG state is going to use when resumed
		mutable AnimCRCContainer	animCRCs;     // List of animation CRCs
		bool											bAdditive;
		bool											bNoAnimCamera; // It won't trigger animation controlled camera even on 1st person

	private:
		void UpdateRequestedAnimStatus() const; // Checks if the requested anim has been loaded and calls OnAnimLoaded() if it is
		void OnTimer(uk pUserData, IGameFramework::TimerID handler) const;
		void OnAnimLoaded() const;

		mutable i16										m_iNextAnimIndex;
		mutable u32									m_nextAnimCRC;
		mutable u32									m_requestedAnimCRC;
		mutable IGameFramework::TimerID	m_iTimerHandle;
	};
	DECLARE_SHARED_POINTERS(SReactionAnim);

	//////////////////////////////////////////////////////////////////////////
	struct SAnimGraphReaction
	{
		struct SVariationData
		{
			SVariationData() {}
			SVariationData(tukk szName, tukk szValue) : sName(szName), sValue(szValue) {}

			void GetMemoryUsage(IDrxSizer * s) const
			{
				s->AddObject(sName);
				s->AddObject(sValue);
			}

			string sName;
			string sValue;
		};
		typedef std::vector<SVariationData> VariationsContainer;

		void Reset();

		string							sAGInputValue;				// Value that will be set in the signal input on the Animation Graph
		VariationsContainer	variations;						// List of all the variations
	};
	//////////////////////////////////////////////////////////////////////////
	struct SMannequinData
	{
		enum EActionType
		{
			EActionType_Fragment,
			EActionType_Coop,
			EActionType_FragTagCopyingFragment,
			EActionType_DoNothing,
			EActionType_Last = EActionType_DoNothing,

			EActionType_Invalid,
		};

		void Initialize( const IActionController* piActionController );
		void AddDB( const IActionController* piActionController, const IAnimationDatabase* piAnimationDatabase ) const;
		i32	 GetNextReactionAnimIndex() const;
		i32	 GetCurrentReactionAnimIndex() const;
		void RequestNextAnim( const IActionController* piActionController ) const;
		void ReleaseRequestedAnims();
		bool IsCurrentFragmentLoaded() const;

		SMannequinData() { Reset(); }
		void Reset();

		TagState tagState;
		mutable u32 m_iNextOptionIndex;
		EActionType actionType;

		u32 m_numOptions;

		// Added at the end of C3 because MP hitdeath works so differently the new precaching system was crashing.
		// Essentially the way it worked before pre-caching was added.
		mutable u32 m_animIndexMP;

	private:

		void UpdateRequestedAnimStatus() const; // Checks if the requested anim has been loaded and calls OnAnimLoaded() if it is
		void OnTimer(uk pUserData, IGameFramework::TimerID handler) const;
		void OnAnimLoaded() const;

		mutable IGameFramework::TimerID	m_iTimerHandle;
		mutable std::shared_ptr<CFragmentCache> m_pRequestedFragment;
		mutable std::shared_ptr<CFragmentCache> m_pCurrentFragment;
	};


	typedef VectorSet<i32>			IdContainer;
	typedef VectorSet<float>		ThresholdsContainer;

	//////////////////////////////////////////////////////////////////////////
	struct SValidationParams
	{
		SValidationParams();
		void Reset(); 
		void GetMemoryUsage(IDrxSizer * s) const;

		ScriptTablePtr			validationParamsScriptTable;	// Holds the pointer to the script table holding the validation information

		string							sCustomValidationFunc;// Name of the customized validation func (if present)
		float								fMinimumSpeedAllowed;	// Minimum speed of the actor receiving the hit for this reaction to be valid
		float								fMaximumSpeedAllowed; // Maximum speed of the actor receiving the hit for this reaction to be valid
		IdContainer					allowedPartIds;				// set of partIds where the hit is allowed to impact for this reaction to be valid
		ECardinalDirection	shotOrigin;						// cardinal direction where the shot came from
		ECardinalDirection	movementDir;					// cardinal direction the player is moving towards
		float								fProbability;					// decimal percentage of probability for this reaction to happen
		IdContainer					allowedStances;				// this reaction is only allowed on these stances
		IdContainer					allowedHitTypes;			// this reaction is only allowed from one of these hit types
		IdContainer					allowedProjectiles;		// this reaction is only allowed when caused by one of these projectile class Ids
		IdContainer					allowedWeapons;				// this reaction is only allowed when caused by one of these weapon class Ids
		float								fMinimumDamageAllowed;	// Minimum damage for this reaction to be valid
		float								fMaximumDamageAllowed;	// Maximum damage for this reaction to be valid
		ThresholdsContainer	healthThresholds;			// The hit reaction is only allowed when it causes the health of the character to go past one of the health values in this container
		u32				destructibleEvent;		// Last destructible event CRC32 generated by the destructible parts system
		float								fMinimumDistance;			// Minimum distance from the hit source allowed
		float								fMaximumDistance;			// Maximum distance from the hit source allowed
		bool								bAllowOnlyWhenUsingMountedItems;	// If TRUE, the reaction will only be valid if the guy is using a mounted item, if FALSE, it will be valid anytime
		EAirState						airState;							// Only allow if actor is in the air or on the ground.
	};
	typedef std::vector<SValidationParams>	ValidationParamsList;

	enum Flags
	{
		OrientateToHitDir											= BIT(0), // If marked, the entity orientates towards the shot origin before animation-based reactions
		CollisionCheckIntersectionWithGround	= BIT(1), // If marked and detecting collisions, the checks will try to get collisions with the ground
		SleepRagdoll													= BIT(2), // If marked if the ragdoll is enabled sometime in this reaction, it will be slept right away
		OrientateToMovementDir								= BIT(3), // If marked, the entity orientates towards the movement direction before animation-based reactions
		NoRagdollOnEnd												= BIT(4), // If marked, death reactions don't end on ragdoll
		ReactionFinishesNotAiming							= BIT(5), // If marked we don't force aiming at the end on animation-based reactions
		ReactionsForbidden										= BIT(6), // If marked further hit/death reactions during this reaction are forbidden
		TriggerRagdollAfterSerializing				= BIT(7), // If marked it will force the ragdoll after loading game
	};

	SReactionParams();
	void Reset();

	void GetMemoryUsage(IDrxSizer * s) const;

	ScriptTablePtr			reactionScriptTable;	// Holds the pointer to the script table holding the reaction information

	// Validation attributes
	ValidationParamsList	validationParams;

	// Execution attributes
	string							sCustomExecutionFunc;	// Name of the customized execution func (if present)
	string							sCustomExecutionEndFunc; // Name of the customized execution func when the reaction finishes (if present)
	string							sCustomAISignal;			// Signal to send to the actor's AI when reaction starts playing
	SAnimGraphReaction	agReaction;						// Parameters for the anim-graph based reaction
	SReactionAnimPtr		reactionAnim;					// Parameters for the reaction anim used for the default execution
	SMannequinData			mannequinData;				// Pre-calculated tag state for cry mannequin
	float								orientationSnapAngle;	// The angle in radians that the animation ought to play in relative to the hit direction
	Vec3								endVelocity;					// Vector with the velocity (in local space) the actor will be forced to have at the end of the reaction
	bool								bPauseAI;							// while playing this reaction the AI is disabled
	u8								flags;								// several flags (see SReactionParams::Flags enum above)
	u8				reactionOnCollision;	// Specifies if we want reaction on collision or not, and which reaction (collisions on death reactions are always ragdoll)
};

//////////////////////////////////////////////////////////////////////////
// Struct holding configuration for each file
//////////////////////////////////////////////////////////////////////////
struct SHitDeathReactionsConfig
{
	SHitDeathReactionsConfig() : iCollisionBoneId(-1), fCollisionRadius(0.6f), fCollisionVerticalOffset(0.5f), 
		fCollMaxHorzAngleSin(0.342f), fCollMaxMovAngleCos(0.7071f), fCollReactionStartDist(0.4f), fMaximumReactionTime(4.0f),
	manqTargetCRC(0), piOptionalAnimationADB(NULL), fragmentID(FRAGMENT_ID_INVALID), fEndRagdollTime(-1.0f) {}

	void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
	}

	i32			iCollisionBoneId;					// Id of the bone the collision volume used for collisions with the environment is centered on
	float		fCollisionRadius;					// Radius of the collision volume used for collisions with the environment
	float		fCollisionVerticalOffset;	// Vertical offset applied to the collision volume position
	float		fCollMaxHorzAngleSin;			// Sin of the maximum angle of the collision normal respect the horizontal (x-y) plane
	float		fCollMaxMovAngleCos;			// Cos of the maximum angle of the collision normal respect the movement direction
	float		fCollReactionStartDist;		// distance from the collision point (and parallel to the normal) where the collision reaction starts
	float		fMaximumReactionTime;			// Maximum time a reaction will last. Used as failsafe when reactions can't communicate their end properly
	float		fEndRagdollTime;					// End time for the ragdoll - we force it off if this is set.
	u32	manqTargetCRC;						// The target tag for Coop
	u32  fragmentID;								// Mannequin "hitDeath" fragment ID
	const IAnimationDatabase* piOptionalAnimationADB; // The optional target adb for the slave.
};

DECLARE_SHARED_POINTERS(CHitDeathReactions);

// SReactionParams related typedefs
typedef std::vector<SReactionParams>									ReactionsContainer;
DECLARE_SHARED_POINTERS(ReactionsContainer);
DECLARE_SHARED_POINTERS(SHitDeathReactionsConfig);

#endif // __HIT_DEATH_REACTIONS_DEFS_H
