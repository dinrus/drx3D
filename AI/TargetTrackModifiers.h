// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Definitions for various modifiers to target tracks

   -------------------------------------------------------------------------
   История:
   - 02:08:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __TARGET_TRACK_MODIFIERS_H__
#define __TARGET_TRACK_MODIFIERS_H__

#include <drx3D/AI/TargetTrackCommon.h>

struct SStimulusInvocation;

struct ITargetTrackModifier
{
	virtual ~ITargetTrackModifier()
	{
	}

	virtual u32 GetUniqueId() const = 0;

	// Returns if this modifier matches the given xml tag
	virtual bool        IsMatchingTag(tukk szTag) const = 0;
	virtual char const* GetTag() const = 0;

	// Returns the modifier value
	virtual float GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                          const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                          const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const = 0;
};

class CTargetTrackDistanceModifier : public ITargetTrackModifier
{
public:
	CTargetTrackDistanceModifier();
	virtual ~CTargetTrackDistanceModifier();

	enum { UNIQUE_ID = 1 };
	virtual u32      GetUniqueId() const { return UNIQUE_ID; }

	virtual bool        IsMatchingTag(tukk szTag) const;
	virtual char const* GetTag() const;
	virtual float       GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                                const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                                const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const;
};

class CTargetTrackHostileModifier : public ITargetTrackModifier
{
public:
	CTargetTrackHostileModifier();
	virtual ~CTargetTrackHostileModifier();

	enum { UNIQUE_ID = 2 };
	virtual u32      GetUniqueId() const { return UNIQUE_ID; }

	virtual bool        IsMatchingTag(tukk szTag) const;
	virtual char const* GetTag() const;
	virtual float       GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                                const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                                const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const;
};

class CTargetTrackClassThreatModifier : public ITargetTrackModifier
{
public:
	CTargetTrackClassThreatModifier();
	virtual ~CTargetTrackClassThreatModifier();

	enum { UNIQUE_ID = 3 };
	virtual u32      GetUniqueId() const { return UNIQUE_ID; }

	virtual bool        IsMatchingTag(tukk szTag) const;
	virtual char const* GetTag() const;
	virtual float       GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                                const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                                const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const;
};

class CTargetTrackDistanceIgnoreModifier : public ITargetTrackModifier
{
public:
	CTargetTrackDistanceIgnoreModifier();
	virtual ~CTargetTrackDistanceIgnoreModifier();

	enum { UNIQUE_ID = 4 };
	virtual u32      GetUniqueId() const { return UNIQUE_ID; }

	virtual bool        IsMatchingTag(tukk szTag) const;
	virtual char const* GetTag() const;
	virtual float       GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                                const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                                const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const;
};

class CTargetTrackPlayerModifier : public ITargetTrackModifier
{
public:
	enum { UNIQUE_ID = 5 };
	virtual u32      GetUniqueId() const { return UNIQUE_ID; }

	virtual bool        IsMatchingTag(tukk szTag) const;
	virtual char const* GetTag() const;
	virtual float       GetModValue(const CTargetTrack* pTrack, TargetTrackHelpers::EAIEventStimulusType stimulusType,
	                                const Vec3& vPos, const TargetTrackHelpers::SEnvelopeData& envelopeData,
	                                const TargetTrackHelpers::STargetTrackModifierConfig& modConfig) const;
};

#endif //__TARGET_TRACK_MODIFIERS_H__
