// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FragmentTrack_h__
#define __FragmentTrack_h__

#include "IDrxMannequin.h"
#include "MannequinBase.h"
#include "SequencerTrack.h"

#if _MSC_VER > 1000
	#pragma once
#endif

const float LOCK_TIME_DIFF = 0.1f;

//////////////////////////////////////////////////////////////////////////
struct CFragmentKey
	: public CSequencerKey
{
	CFragmentKey()
		:
		context(NULL),
		fragmentID(FRAGMENT_ID_INVALID),
		fragOptionIdx(OPTION_IDX_RANDOM),
		scopeMask(0),
		sharedID(0),
		historyItem(HISTORY_ITEM_INVALID),
		clipDuration(0.0f),
		transitionTime(0.0f),
		tranSelectTime(0.0f),
		tranStartTime(0.0f),
		tranStartTimeValue(0.0f),
		tranStartTimeRelative(0.0f),
		tranFragFrom(FRAGMENT_ID_INVALID),
		tranFragTo(FRAGMENT_ID_INVALID),
		hasFragment(false),
		isLooping(false),
		trumpPrevious(false),
		transition(false),
		tranFlags(0),
		fragIndex(0)
	{
	}
	SScopeContextData* context;
	FragmentID         fragmentID;
	SFragTagState      tagStateFull;
	SFragTagState      tagState;
	u32             fragOptionIdx;
	ActionScopes       scopeMask;
	u32             sharedID;
	u32             historyItem;
	float              clipDuration;
	float              transitionTime;
	float              tranSelectTime;
	float              tranStartTime;
	float              tranStartTimeValue;
	float              tranStartTimeRelative;
	FragmentID         tranFragFrom;
	FragmentID         tranFragTo;
	SFragTagState      tranTagFrom;
	SFragTagState      tranTagTo;
	SFragmentBlendUid  tranBlendUid;
	float              tranLastClipEffectiveStart;
	float              tranLastClipDuration;
	bool               hasFragment;
	bool               isLooping;
	bool               trumpPrevious;
	bool               transition;
	u8              tranFlags;
	u8              fragIndex;

	void SetFromQuery(const SBlendQueryResult& query)
	{
		transition = true;
		tranFragFrom = query.fragmentFrom;
		tranFragTo = query.fragmentTo;
		tranTagFrom = query.tagStateFrom;
		tranTagTo = query.tagStateTo;
		tranBlendUid = query.blendUid;
		tranFlags = query.pFragmentBlend->flags;
	}
};

//////////////////////////////////////////////////////////////////////////
class CFragmentTrack : public TSequencerTrack<CFragmentKey>
{
public:

	enum ESecondaryKey
	{
		eSK_SELECT_TIME = 1,
		eSK_START_TIME  = 2
	};

	CFragmentTrack(SScopeData& scopeData, EMannequinEditorMode editorMode);

	virtual ColorB            GetColor() const;
	virtual const SKeyColour& GetKeyColour(i32 key) const;
	virtual const SKeyColour& GetBlendColour(i32 key) const;

	void                      SetHistory(SFragmentHistoryContext& history);

	enum EMenuOptions
	{
		EDIT_FRAGMENT = KEY_MENU_CMD_BASE,
		EDIT_TRANSITION,
		INSERT_TRANSITION,
		FIND_FRAGMENT_REFERENCES,
		FIND_TAG_REFERENCES,
	};

	virtual i32     GetNumSecondarySelPts(i32 key) const;
	virtual i32     GetSecondarySelectionPt(i32 key, float timeMin, float timeMax) const;
	virtual i32     FindSecondarySelectionPt(i32& key, float timeMin, float timeMax) const;

	virtual void    SetSecondaryTime(i32 key, i32 idx, float time);
	virtual float   GetSecondaryTime(i32 key, i32 idx) const;

	virtual bool    CanEditKey(i32 key) const;
	virtual bool    CanMoveKey(i32 key) const;
	virtual bool    CanAddKey(float time) const;
	virtual bool    CanRemoveKey(i32 key) const;

	i32             GetNextFragmentKey(i32 key) const;
	i32             GetPrecedingFragmentKey(i32 key, bool includeTransitions) const;
	i32             GetParentFragmentKey(i32 key) const;

	virtual CString GetSecondaryDescription(i32 key, i32 idx) const;

	virtual void    InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void    ClearKeyMenuOptions(CMenu& menu, i32 keyID)
	{
	}
	virtual void  OnKeyMenuOption(i32 menuOption, i32 keyID);

	void          GetKeyInfo(i32 key, tukk & description, float& duration);
	virtual float GetKeyDuration(i32k key) const;
	void          SerializeKey(CFragmentKey& key, XmlNodeRef& keyNode, bool bLoading);

	void          SelectKey(i32 keyID, bool select);
	void          CloneKey(i32 nKey, const CFragmentKey& key);
	void          DistributeSharedKey(i32 keyID);

	virtual void  SetKey(i32 index, CSequencerKey* _key);

	//! Set time of specified key.
	virtual void      SetKeyTime(i32 index, float time);

	SScopeData&       GetScopeData();
	const SScopeData& GetScopeData() const;

private:
	SScopeData&              m_scopeData;
	SFragmentHistoryContext* m_history;
	EMannequinEditorMode     m_editorMode;

	static u32            s_sharedKeyID;
	static bool              s_distributingKey;
};

struct SAnimCache
{
	SAnimCache()
		:
		animID(-1),
		pAnimSet(NULL)
	{
	}

	SAnimCache(const SAnimCache& animCache)
	{
		if (animCache.animID >= 0)
		{
			animCache.pAnimSet->AddRef(animCache.animID);
		}
		animID = animCache.animID;
		pAnimSet = animCache.pAnimSet;
	}

	~SAnimCache()
	{
		if (animID >= 0)
		{
			pAnimSet->Release(animID);
		}
	}

	SAnimCache& operator=(const SAnimCache& source)
	{
		Set(source.animID, source.pAnimSet);

		return *this;
	}

	void Set(i32 _animID, IAnimationSet* _pAnimSet)
	{
		if (animID >= 0)
		{
			pAnimSet->Release(animID);
		}

		if (_animID >= 0)
		{
			_pAnimSet->AddRef(_animID);
		}
		animID = _animID;
		pAnimSet = _pAnimSet;
	}

	i32                       animID;
	_smart_ptr<IAnimationSet> pAnimSet;
};

//////////////////////////////////////////////////////////////////////////
struct CClipKey : public CSequencerKey
{
	CClipKey();
	~CClipKey();

	void Set(const SAnimClip& animClip, IAnimationSet* pAnimSet, const EClipType transitionType[SFragmentData::PART_TOTAL]);
	void SetupAnimClip(SAnimClip& animClip, float lastTime, i32 fragPart);

	// Change animation and automatically update duration & anim location flags
	void         SetAnimation(tukk szAnimName, IAnimationSet* pAnimSet);
	bool         IsAdditive() const { return animIsAdditive; }
	bool         IsLMG() const      { return animIsLMG; }
	tukk  GetDBAPath() const;

	virtual void UpdateFlags();

	// Returns the time the animation asset "starts" (in seconds),
	// taking into account that we can shift the animation clip back in time
	// using startTime
	ILINE float GetEffectiveAssetStartTime() const
	{
		return m_time - startTime;
	}

	// Takes into account both playbackSpeed and startTime
	ILINE float ConvertTimeToUnclampedNormalisedTime(const float timeSeconds) const
	{
		const float timeFromEffectiveStart = timeSeconds - GetEffectiveAssetStartTime();

		const float oneLoopDuration = GetOneLoopDuration();
		if (oneLoopDuration < FLT_EPSILON)
			return 0.0f;

		const float unclampedNormalisedTime = timeFromEffectiveStart / oneLoopDuration;
		return unclampedNormalisedTime;
	}

	// Takes into account both playbackSpeed and startTime
	ILINE float ConvertTimeToNormalisedTime(const float timeSeconds) const
	{
		const float unclampedNormalisedTime = ConvertTimeToUnclampedNormalisedTime(timeSeconds);
		const float normalisedTime = unclampedNormalisedTime - (i32)unclampedNormalisedTime;

		return normalisedTime;
	}

	// Takes into account both playbackSpeed and startTime
	ILINE float ConvertUnclampedNormalisedTimeToTime(const float unclampedNormalisedTime) const
	{
		const float timeFromStart = GetOneLoopDuration() * unclampedNormalisedTime;
		const float timeSeconds = timeFromStart + GetEffectiveAssetStartTime();

		return timeSeconds;
	}

	// Takes into account both playbackSpeed and startTime
	ILINE float ConvertNormalisedTimeToTime(const float normalisedTime, i32k loopIndex = 0) const
	{
		assert(normalisedTime >= 0);
		assert(normalisedTime <= 1.0f);

		const float unclampedNormalisedTime = normalisedTime + loopIndex;
		const float timeSeconds = ConvertUnclampedNormalisedTimeToTime(unclampedNormalisedTime);

		return timeSeconds;
	}

	// Returns the duration in seconds of one loop of the underlying animation.
	// Whether or not the clip is marked as 'looping', it will only count one loop.
	// Takes into account playbackSpeed.
	ILINE float GetOneLoopDuration() const
	{
		if (duration < FLT_EPSILON)
		{
			// when duration is 0, for example when it's a None clip,
			// we want to treat it as 0-length regardless of the playbackSpeed
			return 0.0f;
		}

		if (playbackSpeed < FLT_EPSILON)
		{
			return FLT_MAX;
		}

		return duration / playbackSpeed;
	}

	ILINE float GetDuration() const
	{
		return (duration / max(0.1f, playbackSpeed)) - startTime;
	}

	ILINE float GetBlendOutTime() const
	{
		const float cachedDuration = GetDuration();
		return clamp_tpl(cachedDuration - blendOutDuration, 0.0f, cachedDuration);
	}

	ILINE float GetAssetDuration() const
	{
		return duration;
	}

	ILINE float GetVariableDurationDelta() const
	{
		if (referenceLength > 0.0f)
		{
			return (duration / max(0.1f, playbackSpeed)) - referenceLength;
		}
		else
		{
			return 0.0f;
		}
	}

	ILINE void Serialize(XmlNodeRef& keyNode, bool bLoading)
	{
		// Variables not serialized:
		// SAnimCache m_animCache; Set on anim set
		// bool animIsAdditive; Set on anim set
		// bool animIsLMG; Set on anim set
		// CString animExtension; Set on anim set
		// const IAnimationSet* m_animSet; Set on anim set

		if (bLoading)
		{
			keyNode->getAttr("historyItem", historyItem);
			keyNode->getAttr("startTime", startTime);
			keyNode->getAttr("playbackSpeed", playbackSpeed);
			keyNode->getAttr("playbackWeight", playbackWeight);
			keyNode->getAttr("blend", blendDuration);
			keyNode->getAttr("blendOutDuration", blendOutDuration);
			for (i32 i = 0; i < MANN_NUMBER_BLEND_CHANNELS; ++i)
			{
				char name[16];
				drx_sprintf(name, "blendChannels%d", i);
				keyNode->getAttr(name, blendChannels[i]);
			}
			keyNode->getAttr("animFlags", animFlags);
			keyNode->getAttr("jointMask", jointMask);
			//			keyNode->getAttr("align",alignToPrevious);
			keyNode->getAttr("clipType", clipType);
			keyNode->getAttr("blendType", blendType);
			keyNode->getAttr("fragIndexBlend", fragIndexBlend);
			keyNode->getAttr("fragIndexMain", fragIndexMain);
			keyNode->getAttr("duration", duration);
			keyNode->getAttr("referenceLength", referenceLength);
		}
		else
		{
			keyNode->setAttr("historyItem", historyItem);
			keyNode->setAttr("startTime", startTime);
			keyNode->setAttr("playbackSpeed", playbackSpeed);
			keyNode->setAttr("playbackWeight", playbackWeight);
			keyNode->setAttr("blend", blendDuration);
			keyNode->setAttr("blendOutDuration", blendOutDuration);
			for (i32 i = 0; i < MANN_NUMBER_BLEND_CHANNELS; ++i)
			{
				char name[16];
				drx_sprintf(name, "blendChannels%d", i);
				keyNode->setAttr(name, blendChannels[i]);
			}
			keyNode->setAttr("animFlags", animFlags);
			keyNode->setAttr("jointMask", jointMask);
			//			keyNode->setAttr("align",alignToPrevious);
			keyNode->setAttr("clipType", clipType);
			keyNode->setAttr("blendType", blendType);
			keyNode->setAttr("fragIndexBlend", fragIndexBlend);
			keyNode->setAttr("fragIndexMain", fragIndexMain);
			keyNode->setAttr("referenceLength", referenceLength);
		}
	}

	u32   historyItem;
	SAnimRef animRef;
	float    startTime;
	float    playbackSpeed;
	float    playbackWeight;
	float    blendDuration;
	float    blendOutDuration;
	float    blendChannels[MANN_NUMBER_BLEND_CHANNELS];
	i32      animFlags;
	u8    jointMask;
	bool     alignToPrevious;
	u8    clipType;
	u8    blendType;
	u8    fragIndexBlend;
	u8    fragIndexMain;

protected:
	virtual void GetExtensions(std::vector<CString>& extensions, CString& editableExtension) const;

	float                duration;
	float                referenceLength;
	SAnimCache           m_animCache;
	bool                 animIsAdditive;
	bool                 animIsLMG;
	CString              animExtension;
	const IAnimationSet* m_animSet;
};

//////////////////////////////////////////////////////////////////////////
class CClipTrack : public TSequencerTrack<CClipKey>
{
public:
	enum EClipKeySecondarySel
	{
		eCKSS_None = 0,
		eCKSS_BlendIn,
	};

	enum EMenuOptions
	{
		FIND_FRAGMENT_REFERENCES = KEY_MENU_CMD_BASE,
	};

	CClipTrack(SScopeContextData* pContext, EMannequinEditorMode editorMode);

	ColorB                    GetColor() const;
	virtual const SKeyColour& GetKeyColour(i32 key) const;
	virtual const SKeyColour& GetBlendColour(i32 key) const;

	virtual i32               GetNumSecondarySelPts(i32 key) const;
	virtual i32               GetSecondarySelectionPt(i32 key, float timeMin, float timeMax) const;
	virtual i32               FindSecondarySelectionPt(i32& key, float timeMin, float timeMax) const;
	virtual void              SetSecondaryTime(i32 key, i32 idx, float time);
	virtual float             GetSecondaryTime(i32 key, i32 id) const;
	virtual bool              CanMoveSecondarySelection(i32 key, i32 id) const;

	virtual void              InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              ClearKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              OnKeyMenuOption(i32 menuOption, i32 keyID);

	virtual bool              CanAddKey(float time) const;
	virtual bool              CanRemoveKey(i32 key) const;

	virtual bool              CanEditKey(i32 key) const;
	virtual bool              CanMoveKey(i32 key) const;

	virtual i32               CreateKey(float time);

	void                      CheckKeyForSnappingToPrevious(i32 index);

	virtual void              SetKey(i32 index, CSequencerKey* _key);

	virtual void              SetKeyTime(i32 index, float time);

	void                      GetKeyInfo(i32 key, tukk & description, float& duration);
	void                      GetTooltip(i32 key, tukk & description, float& duration);
	virtual float             GetKeyDuration(i32k key) const;
	void                      SerializeKey(CClipKey& key, XmlNodeRef& keyNode, bool bLoading);

	void                      SetMainAnimTrack(bool bSet) { m_mainAnimTrack = bSet; }

	const SScopeContextData*  GetContext() const
	{
		return m_pContext;
	}

protected:

	virtual void OnChangeCallback()
	{
		if (m_pContext)
		{
			m_pContext->changeCount++;
		}
	}

private:
	bool                 m_mainAnimTrack;
	SScopeContextData*   m_pContext;
	EMannequinEditorMode m_editorMode;
};

//////////////////////////////////////////////////////////////////////////
struct CProcClipKey : public CSequencerKey
{
	CProcClipKey();
	IProceduralClipFactory::THash typeNameHash;
	float                         duration;
	float                         blendDuration;
	IProceduralParamsPtr          pParams;
	u32                        historyItem;
	u8                         clipType;
	u8                         blendType;
	u8                         fragIndexMain;
	u8                         fragIndexBlend;

	void FromProceduralEntry(const SProceduralEntry& procClip, const EClipType transitionType[SFragmentData::PART_TOTAL]);
	void ToProceduralEntry(SProceduralEntry& procClip, const float lastTime, i32k fragPart);

	void UpdateDurationBasedOnParams();

	void Serialize(Serialization::IArchive& ar);
};

//////////////////////////////////////////////////////////////////////////
class CProcClipTrack : public TSequencerTrack<CProcClipKey>
{
public:

	CProcClipTrack(SScopeContextData* pContext, EMannequinEditorMode editorMode);

	ColorB                    GetColor() const;
	virtual const SKeyColour& GetKeyColour(i32 key) const;
	virtual const SKeyColour& GetBlendColour(i32 key) const;

	virtual i32               GetNumSecondarySelPts(i32 key) const;
	virtual i32               GetSecondarySelectionPt(i32 key, float timeMin, float timeMax) const;
	virtual i32               FindSecondarySelectionPt(i32& key, float timeMin, float timeMax) const;
	virtual void              SetSecondaryTime(i32 key, i32 idx, float time);
	virtual float             GetSecondaryTime(i32 key, i32 id) const;
	virtual bool              CanMoveSecondarySelection(i32 key, i32 id) const;

	virtual void              InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              ClearKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              OnKeyMenuOption(i32 menuOption, i32 keyID);

	virtual bool              CanAddKey(float time) const;

	virtual bool              CanEditKey(i32 key) const;
	virtual bool              CanMoveKey(i32 key) const;

	virtual i32               CreateKey(float time);

	void                      GetKeyInfo(i32 key, tukk & description, float& duration);
	virtual float             GetKeyDuration(i32k key) const;
	void                      SerializeKey(CProcClipKey& key, XmlNodeRef& keyNode, bool bLoading);

protected:
	virtual void OnChangeCallback()
	{
		if (m_pContext)
		{
			m_pContext->changeCount++;
		}
	}

private:
	SScopeContextData*   m_pContext;
	EMannequinEditorMode m_editorMode;
};

//////////////////////////////////////////////////////////////////////////
struct CTagKey : public CSequencerKey
{
	CTagKey()
		: tagState(TAG_STATE_EMPTY)
	{
	}
	TagState                   tagState;
	DrxStackStringT<char, 128> desc;
};

//////////////////////////////////////////////////////////////////////////
class CTagTrack : public TSequencerTrack<CTagKey>
{
public:

	CTagTrack(const CTagDefinition& tagDefinition);

	void                      GetKeyInfo(i32 key, tukk & description, float& duration);
	virtual float             GetKeyDuration(i32k key) const;

	virtual void              InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              ClearKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              OnKeyMenuOption(i32 menuOption, i32 keyID);
	void                      SerializeKey(CTagKey& key, XmlNodeRef& keyNode, bool bLoading);

	virtual void              SetKey(i32 index, CSequencerKey* _key);

	ColorB                    GetColor() const;
	virtual const SKeyColour& GetKeyColour(i32 key) const;
	virtual const SKeyColour& GetBlendColour(i32 key) const;

private:
	const CTagDefinition& m_tagDefinition;
};

//////////////////////////////////////////////////////////////////////////
struct CTransitionPropertyKey : public CSequencerKey
{
	enum EProp
	{
		eTP_Select,
		eTP_Start,
		eTP_Transition,
		eTP_Count,
	};

	CTransitionPropertyKey()
		: prop(eTP_Select)
		, refTime(0)
		, duration(0)
		, overrideProps(false)
		, toHistoryItem(HISTORY_ITEM_INVALID)
		, sharedId(0)
		, tranFlags(0)
	{
	}
	SBlendQueryResult blend;
	EProp             prop;
	float             refTime;
	float             duration;
	float             prevClipStart;
	float             prevClipDuration;
	float             overrideStartTime;
	float             overrideSelectTime;
	i32               toHistoryItem;
	i32               toFragIndex;
	i32               sharedId;
	u8             tranFlags;
	bool              overrideProps;
};

//////////////////////////////////////////////////////////////////////////
class CTransitionPropertyTrack : public TSequencerTrack<CTransitionPropertyKey>
{
public:

	CTransitionPropertyTrack(SScopeData& scopeData);

	void                          GetKeyInfo(i32 key, tukk & description, float& duration);
	virtual float                 GetKeyDuration(i32k key) const;

	virtual bool                  CanEditKey(i32 key) const;
	virtual bool                  CanMoveKey(i32 key) const;
	virtual bool                  CanAddKey(float time) const { return false; }
	virtual bool                  CanRemoveKey(i32 key) const { return false; }

	virtual void                  InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void                  ClearKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void                  OnKeyMenuOption(i32 menuOption, i32 keyID);
	void                          SerializeKey(CTransitionPropertyKey& key, XmlNodeRef& keyNode, bool bLoading);

	virtual void                  SetKey(i32 index, CSequencerKey* _key);

	ColorB                        GetColor() const;
	virtual const SKeyColour&     GetKeyColour(i32 key) const;
	virtual const SKeyColour&     GetBlendColour(i32 key) const;

	virtual i32k             GetNumBlendsForKey(i32 index) const;
	virtual const SFragmentBlend* GetBlendForKey(i32 index) const;
	virtual const SFragmentBlend* GetAlternateBlendForKey(i32 index, i32 blendIdx) const;
	virtual void                  UpdateBlendForKey(i32 index, SFragmentBlend& blend) const;
protected:
	const SScopeData& m_scopeData;
};

//////////////////////////////////////////////////////////////////////////
struct CParamKey : public CSequencerKey
{
	CParamKey()
		:
		historyItem(HISTORY_ITEM_INVALID),
		isLocation(false)
	{
		parameter.value.SetIdentity();
	}

	SMannParameter parameter;
	CString        name;
	u32         historyItem;
	bool           isLocation;
};

//////////////////////////////////////////////////////////////////////////
class CParamTrack : public TSequencerTrack<CParamKey>
{
public:

	CParamTrack();

	void                      GetKeyInfo(i32 key, tukk & description, float& duration);
	virtual float             GetKeyDuration(i32k key) const;

	virtual void              InsertKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              ClearKeyMenuOptions(CMenu& menu, i32 keyID);
	virtual void              OnKeyMenuOption(i32 menuOption, i32 keyID);
	void                      SerializeKey(CParamKey& key, XmlNodeRef& keyNode, bool bLoading);

	ColorB                    GetColor() const;
	virtual const SKeyColour& GetKeyColour(i32 key) const;
	virtual const SKeyColour& GetBlendColour(i32 key) const;

private:

};

#endif // __FragmentTrack_h__

