// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ACTIONSCOPE_H__
#define __ACTIONSCOPE_H__

#include <drx3D/Act/IDrxMannequin.h>
#include "ActionController.h"

struct SScopeContext
{
	SScopeContext()
		: id(0)
		, mask(0)
		, pDatabase(NULL)
		, pCharInst(NULL)
		, entityId(0)
		, pCachedEntity(NULL)
		, pEnslavedController(NULL)
	{
	}

	void Reset(u32 scopeContextId)
	{
		id = scopeContextId;
		mask = (1 << scopeContextId);
		pDatabase = NULL;
		pCharInst = NULL;
		entityId = 0;
		pCachedEntity = NULL;
		pEnslavedController = NULL;
	}

	// HasInvalidCharInst returns true when
	//   character instance is not NULL
	// and
	//   the character instance is not part of the entity
	bool HasInvalidCharInst() const;

	// HasInvalidEntity returns true when
	//   entityId is not 0
	// and
	//   the cachedEntity is that entity
	bool HasInvalidEntity() const;

	u32                         id;
	u32                         mask;
	const CAnimationDatabase*      pDatabase;
	_smart_ptr<ICharacterInstance> pCharInst;
	EntityId                       entityId;
	IEntity*                       pCachedEntity;
	CActionController*             pEnslavedController;
	TagState                       sharedTags;
	TagState                       setTags;
};

class CActionScope : public IScope
{
public:
	friend class CActionController;

	CActionScope(tukk _name, u32 scopeID, CActionController& actionController, SAnimationContext& _context, SScopeContext& _scopeContext, i32 layer, i32 numLayers, const TagState& additionalTags);

	// -- IScope implementation -------------------------------------------------
	virtual ~CActionScope();

	virtual tukk GetName()
	{
		return m_name.c_str();
	}

	virtual u32 GetID()
	{
		return m_id;
	}

	virtual ICharacterInstance* GetCharInst()
	{
		return m_scopeContext.pCharInst;
	}

	virtual IActionController& GetActionController() const
	{
		return (IActionController&)m_actionController;
	}

	virtual SAnimationContext& GetContext() const
	{
		return m_context;
	}

	virtual u32 GetContextID() const
	{
		return m_scopeContext.id;
	}

	virtual const IAnimationDatabase& GetDatabase() const
	{
		DRX_ASSERT(m_scopeContext.pDatabase);
		return (const IAnimationDatabase&)*m_scopeContext.pDatabase;
	}

	virtual bool HasDatabase() const
	{
		return(m_scopeContext.pDatabase != NULL);
	}

	virtual IEntity& GetEntity() const
	{
		DRX_ASSERT(m_scopeContext.pCachedEntity);
		return *m_scopeContext.pCachedEntity;
	}

	virtual EntityId GetEntityId() const
	{
		return m_scopeContext.entityId;
	}

	virtual u32 GetTotalLayers() const
	{
		return m_numLayers;
	}
	virtual u32 GetBaseLayer() const
	{
		return m_layer;
	}
	virtual IAction* GetAction() const
	{
		return m_pAction.get();
	}

	virtual void              IncrementTime(float timeDelta);

	virtual const CAnimation* GetTopAnim(i32 layer) const;

	virtual CAnimation*       GetTopAnim(i32 layer);

	virtual void              ApplyAnimWeight(u32 layer, float weight);

	virtual bool              IsDifferent(const FragmentID aaID, const TagState& fragmentTags, const TagID subContext = TAG_ID_INVALID) const;

	virtual ILINE FragmentID  GetLastFragmentID() const
	{
		return m_lastFragmentID;
	}

	virtual ILINE const SFragTagState& GetLastTagState() const
	{
		return m_lastFragSelection.tagState;
	}

	virtual float    CalculateFragmentTimeRemaining() const;

	virtual float    CalculateFragmentDuration(const CFragment& fragment) const;

	virtual void     _FlushFromEditor()          { Flush(FM_Normal); }

	virtual float    GetFragmentDuration() const { return m_fragmentDuration; }

	virtual float    GetFragmentTime() const     { return m_fragmentTime; }

	virtual TagState GetAdditionalTags() const   { return m_additionalTags; }

	virtual void     MuteLayers(u32 mutedAnimLayerMask, u32 mutedProcLayerMask);

	// -- ~IScope implementation ------------------------------------------------

	bool NeedsInstall(u32 currentContextMask) const
	{
		return (m_additionalTags != TAG_STATE_EMPTY) || ((currentContextMask & m_scopeContext.mask) == 0);
	}

	bool  InstallAnimation(i32 animID, const DrxCharAnimationParams& animParams);
	bool  InstallAnimation(const SAnimationEntry& animEntry, i32 layer, const SAnimBlend& animBlend);
	void  StopAnimationOnLayer(u32 layer, float blendTime);
	float GetFragmentStartTime() const;
	bool  CanInstall(EPriorityComparison priorityComparison, FragmentID fragID, const SFragTagState& fragTagState, bool isRequeue, float& timeRemaining) const;
	void  Install(IAction& action)
	{
		m_pAction = &action;
		m_speedBias = action.GetSpeedBias();
		m_animWeight = action.GetAnimWeight();
	}
	void         UpdateSequencers(float timePassed);
	void         Update(float timePassed);
	void         ClearSequencers();
	void         Flush(EFlushMethod flushMethod);
	void         QueueAnimFromSequence(u32 layer, u32 pos, bool isPersistent);
	void         QueueProcFromSequence(u32 layer, u32 pos);
	i32          GetNumAnimsInSequence(u32 layer) const;
	bool         PlayPendingAnim(u32 layer, float timePassed = 0.0f);
	bool         PlayPendingProc(u32 layer);
	bool         QueueFragment(FragmentID aaID, const SFragTagState& fragTagState, u32 optionIdx = OPTION_IDX_RANDOM, float startTime = 0.0f, u32 userToken = 0, bool isRootScope = true, bool isHigherPriority = false, bool principleContext = true);
	void         BlendOutFragments();

	ILINE u32 GetContextMask() const
	{
		return m_scopeContext.mask;
	}

	ILINE u32 GetLastOptionIdx() const
	{
		return m_lastFragSelection.optionIdx;
	}

	ILINE bool HasFragment() const
	{
		return ((m_sequenceFlags & eSF_Fragment) != 0);
	}
	ILINE bool HasTransition() const
	{
		return ((m_sequenceFlags & eSF_Transition) != 0);
	}
	ILINE bool HasOutroTransition() const
	{
		return ((m_sequenceFlags & eSF_TransitionOutro) != 0);
	}
	ILINE float GetTransitionDuration() const
	{
		return m_transitionDuration;
	}

	ILINE float GetTransitionOutroDuration() const
	{
		return m_transitionOutroDuration;
	}
	ILINE const IActionPtr& GetPlayingAction()
	{
		return m_pExitingAction ? m_pExitingAction : m_pAction;
	}

	ILINE IActionController* GetEnslavedActionController() const
	{
		return m_scopeContext.pEnslavedController;
	}

	void InstallProceduralClip(const SProceduralEntry& proc, i32 layer, const SAnimBlend& blend, float duration);

	void Pause();
	void Resume(float forcedBlendOutTime, u32 resumeFlags);

private:
	void InitAnimationParams(const SAnimationEntry& animEntry, u32k sequencerLayer, const SAnimBlend& animBlend, DrxCharAnimationParams& paramsOut);
	void FillBlendQuery(SBlendQuery& query, FragmentID fragID, const SFragTagState& fragTagState, bool isHigherPriority, float* pLoopDuration) const;
	void ClipInstalled(u8 clipType);

private:
	CActionScope();
	CActionScope(const CActionScope&);
	CActionScope& operator=(const CActionScope&);

private:

	enum ESequencerFlags
	{
		eSF_Queued      = BIT(0),
		eSF_BlendingOut = BIT(1)
	};

	struct SSequencer
	{

		SSequencer()
			:
			installTime(-1.0f),
			referenceTime(-1.0f),
			savedAnimNormalisedTime(-1),
			pos(0),
			flags(0)
		{
		}

		TAnimClipSequence sequence;
		SAnimBlend        blend;
		float             installTime; // time in seconds until installation
		float             referenceTime;
		float             savedAnimNormalisedTime;
		u8             pos;
		u8             flags;
	};

	struct SProcSequencer
	{
		SProcSequencer()
			:
			installTime(-1.0f),
			pos(0),
			flags(0)
		{
		}

		TProcClipSequence                sequence;
		SAnimBlend                       blend;
		float                            installTime;
		std::shared_ptr<IProceduralClip> proceduralClip;
		u8                            pos;
		u8                            flags;
	};

	string                      m_name;
	u32                      m_id;
	SAnimationContext&          m_context;
	SScopeContext&              m_scopeContext;
	CActionController&          m_actionController;
	i32                         m_layer;
	u32                      m_numLayers;
	SSequencer*                 m_layerSequencers;
	std::vector<SProcSequencer> m_procSequencers;
	float                       m_speedBias;
	float                       m_animWeight;
	float                       m_timeIncrement;
	TagState                    m_additionalTags;
	mutable TagState            m_cachedFragmentTags;
	mutable TagState            m_cachedContextStateMask;
	mutable FragmentID          m_cachedaaID;
	mutable u32              m_cachedTagSetIdx;
	FragmentID                  m_lastFragmentID;
	SFragmentSelection          m_lastFragSelection;
	SFragTagState               m_lastQueueTagState;
	u32                      m_sequenceFlags;
	float                       m_fragmentTime;
	float                       m_fragmentDuration;
	float                       m_transitionOutroDuration;
	float                       m_transitionDuration;
	float                       m_blendOutDuration;
	EClipType                   m_partTypes[SFragmentData::PART_TOTAL];

	float                       m_lastNormalisedTime;
	float                       m_normalisedTime;

	IActionPtr                  m_pAction;
	IActionPtr                  m_pExitingAction;
	u32                      m_userToken; // token that will be passed when installing animations

	u32                      m_mutedAnimLayerMask;
	u32                      m_mutedProcLayerMask;

	bool                        m_isOneShot;
	bool                        m_fragmentInstalled;
};

#endif //!__ACTIONSCOPE_H__
