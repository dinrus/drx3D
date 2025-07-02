// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

// Define if target track should determine the AI threat level for you
#define TARGET_TRACK_DOTARGETTHREAT

// Define if target track should determine the AI target type for you
#define TARGET_TRACK_DOTARGETTYPE

#ifdef DRXAISYS_DEBUG
	#define TARGET_TRACK_DEBUG
#endif

#include <drx3D/AI/ITargetTrackUpr.h>

class CTargetTrack;
struct ITargetTrackModifier;

namespace TargetTrackHelpers
{
// Desired target selection methods
enum EDesiredTargetMethod
{
	// Selection types
	eDTM_Select_Highest = 0x01,
	eDTM_Select_Lowest  = 0x02,
	eDTM_SELECTION_MASK = 0x0F,

	// Selection filters
	eDTM_Filter_LimitDesired    = 0x10,   // Try not to select dead targets if possible
	eDTM_Filter_LimitPotential  = 0x20,   // Check the target limits and obey if possible
	eDTM_Filter_CanAquireTarget = 0x40,   // Check the result of the CanAcquireTarget helper
	eDTM_FILTER_MASK            = 0xF0,

	eDTM_COUNT,
};

// Information related to an envelope
struct SEnvelopeData
{
	float m_fCurrentValue;
	float m_fStartTime;
	float m_fLastInvokeTime;
	float m_fLastRunningValue;
	float m_fLastReleasingValue;
	bool  m_bReinvoked;

	SEnvelopeData();
};

// Describes an incoming stimulus to be handled
struct STargetTrackStimulusEvent
{
	string               m_sStimulusName;
	Vec3                 m_vTargetPos;
	tAIObjectID          m_ownerId;
	tAIObjectID          m_targetId;
	EAITargetThreat      m_eTargetThreat;
	EAIEventStimulusType m_eStimulusType;

	STargetTrackStimulusEvent(tAIObjectID ownerId);
	STargetTrackStimulusEvent(tAIObjectID ownerId, tAIObjectID targetId, tukk szStimulusName, const SStimulusEvent& eventInfo);
};

// Describes a registered pulse for a stimulus configuration
struct STargetTrackPulseConfig
{
	string m_sPulse;
	float  m_fValue;
	float  m_fDuration;
	bool   m_bInherited;

	STargetTrackPulseConfig();
	STargetTrackPulseConfig(tukk szPulse, float fValue, float fDuration);
	STargetTrackPulseConfig(const STargetTrackPulseConfig& other, bool bInherited = false);
};

// Describes a registered modifier for a stimulus configuration
struct STargetTrackModifierConfig
{
	u32 m_uId;
	float  m_fValue;
	float  m_fLimit;
	bool   m_bInherited;

	STargetTrackModifierConfig();
	STargetTrackModifierConfig(u32 uId, float fValue, float fLimit);
	STargetTrackModifierConfig(const STargetTrackModifierConfig& other, bool bInherited = false);
};

// Describes a registered stimulus for a configuration
struct STargetTrackStimulusConfig
{
	// ADSR values not used should be marked with this
	static const float INVALID_VALUE;

	typedef VectorMap<u32, STargetTrackPulseConfig> TPulseContainer;
	TPulseContainer m_pulses;

	typedef VectorMap<u32, STargetTrackModifierConfig> TModifierContainer;
	TModifierContainer m_modifiers;

	typedef VectorMap<EAITargetThreat, float> TThreatLevelContainer;
	TThreatLevelContainer m_threatLevels;

	string                m_sStimulus;
	float                 m_fPeak;
	float                 m_fAttack;
	float                 m_fDecay;
	float                 m_fSustainRatio;
	float                 m_fRelease;
	float                 m_fIgnore;
	bool                  m_bHostileOnly;

	// Mask to state which properties where inherited
	enum EInheritanceMask
	{
		eIM_Peak    = 0x01,
		eIM_Attack  = 0x02,
		eIM_Decay   = 0x04,
		eIM_Sustain = 0x08,
		eIM_Release = 0x10,
		eIM_Ignore  = 0x20,
	};
	u8 m_ucInheritanceMask;

	STargetTrackStimulusConfig();
	STargetTrackStimulusConfig(tukk szStimulus, bool bHostileOnly, float fPeak, float fSustainRatio, float fAttack, float fDecay, float fRelease, float fIgnore);
	STargetTrackStimulusConfig(const STargetTrackStimulusConfig& other, bool bInherited = false);
};

// Describes a registered configuration
struct STargetTrackConfig
{
	typedef std::map<u32, STargetTrackStimulusConfig> TStimulusContainer;
	TStimulusContainer m_stimuli;
	string             m_sName;
	string             m_sTemplate;

	// Helper to know when template values have been applied
	bool m_bTemplateApplied;

	STargetTrackConfig();
	STargetTrackConfig(tukk szName);
};

// Interface for accessing the target track pool
struct ITargetTrackPoolProxy
{
	virtual ~ITargetTrackPoolProxy(){}
	virtual CTargetTrack* GetUnusedTargetTrackFromPool() = 0;
	virtual void          AddTargetTrackToPool(CTargetTrack* pTrack) = 0;
};

// Interface for accessing target track configurations and stimulus configurations
struct ITargetTrackConfigProxy
{
	virtual ~ITargetTrackConfigProxy(){}
	virtual bool                        GetTargetTrackConfig(u32 uNameHash, STargetTrackConfig const*& pOutConfig) const = 0;
	virtual bool                        GetTargetTrackStimulusConfig(u32 uNameHash, u32 uStimulusHash, STargetTrackStimulusConfig const*& pOutConfig) const = 0;
	virtual const ITargetTrackModifier* GetTargetTrackModifier(u32 uId) const = 0;

	virtual void                        ModifyTargetThreat(IAIObject& ownerAI, IAIObject& targetAI, const ITargetTrack& track, float& outThreatRatio, EAITargetThreat& outThreat) const = 0;
};
}

//! \endcond