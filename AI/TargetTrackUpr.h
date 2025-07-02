// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages and updates target track groups for agents to
        determine which target an agent should select

   -------------------------------------------------------------------------
   История:
   - 02:01:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __TARGET_TRACK_MANAGER_H__
#define __TARGET_TRACK_MANAGER_H__

#include <drx3D/AI/ITargetTrackUpr.h>
#include <drx3D/AI/TargetTrackCommon.h>

class CTargetTrackGroup;
class CTargetTrack;
struct ITargetTrackModifier;

struct SAIEVENT;
struct SAIPotentialTarget;

class CTargetTrackUpr : public ITargetTrackUpr
{
public:
	CTargetTrackUpr();
	~CTargetTrackUpr();

	void Init();
	void Shutdown();
	void Reset(IAISystem::EResetReason reason);
	void DebugDraw();
	bool ReloadConfig();
	void OnObjectRemoved(CAIObject* pObject);
	void Serialize(TSerialize ser);

	bool IsEnabled() const;

	// Threat modifier
	virtual void                SetTargetTrackThreatModifier(ITargetTrackThreatModifier* pModifier);
	virtual void                ClearTargetTrackThreatModifier();
	ITargetTrackThreatModifier* GetTargetTrackThreatModifier() const { return m_pThreatModifier; }

	// Target group accessors
	virtual bool  SetTargetClassThreat(tAIObjectID aiObjectId, float fClassThreat);
	virtual float GetTargetClassThreat(tAIObjectID aiObjectId) const;
	virtual i32   GetTargetLimit(tAIObjectID aiObjectId) const;

	// Agent registration to use target tracks
	bool RegisterAgent(tAIObjectID aiObjectId, tukk szConfig, i32 nTargetLimit = 0);
	bool UnregisterAgent(tAIObjectID aiObjectId);
	bool ResetAgent(tAIObjectID aiObjectId);
	bool SetAgentEnabled(tAIObjectID aiObjectId, bool bEnable);

	// Incoming stimulus handling
	virtual bool HandleStimulusEventInRange(tAIObjectID aiTargetId, tukk szStimulusName, const TargetTrackHelpers::SStimulusEvent& eventInfo, float fRadius);
	virtual bool HandleStimulusEventForAgent(tAIObjectID aiAgentId, tAIObjectID aiTargetId, tukk szStimulusName, const TargetTrackHelpers::SStimulusEvent& eventInfo);
	virtual bool TriggerPulse(tAIObjectID aiObjectId, tAIObjectID targetId, tukk szStimulusName, tukk szPulseName);

	bool         HandleStimulusFromAIEvent(tAIObjectID aiObjectId, const SAIEVENT* pAIEvent, TargetTrackHelpers::EAIEventStimulusType eType);

	// Outgoing desired target handling
	void   Update(tAIObjectID aiObjectId);
	void   ShareFreshestTargetData();
	void   PullDownThreatLevel(const tAIObjectID aiObjectIdForTargetTrackGroup, const EAITargetThreat maxAllowedThreat);
	bool   GetDesiredTarget(tAIObjectID aiObjectId, u32 uDesiredTargetMethod, CWeakRef<CAIObject>& outTarget, SAIPotentialTarget*& pOutTargetInfo);
	u32 GetBestTargets(tAIObjectID aiObjectId, u32 uDesiredTargetMethod, tAIObjectID* bestTargets, u32 maxCount);
	i32    GetDesiredTargetCount(tAIObjectID aiTargetId, tAIObjectID aiIgnoreId = 0) const;
	i32    GetPotentialTargetCount(tAIObjectID aiTargetId, tAIObjectID aiIgnoreId = 0) const;
	i32    GetPotentialTargetCountFromFaction(tAIObjectID aiTargetId, tukk factionName, tAIObjectID aiIgnoreId = 0) const;

private:
	// Target track pool management
	CTargetTrack* GetUnusedTargetTrackFromPool();
	void          AddTargetTrackToPool(CTargetTrack* pTrack);

	class CTargetTrackPoolProxy : public TargetTrackHelpers::ITargetTrackPoolProxy
	{
	public:
		CTargetTrackPoolProxy(CTargetTrackUpr* pUpr) : m_pUpr(pUpr)
		{
			assert(m_pUpr);
		}

		virtual CTargetTrack* GetUnusedTargetTrackFromPool()
		{
			return m_pUpr->GetUnusedTargetTrackFromPool();
		}

		virtual void AddTargetTrackToPool(CTargetTrack* pTrack)
		{
			m_pUpr->AddTargetTrackToPool(pTrack);
		}

	private:
		CTargetTrackUpr* m_pUpr;
	};

private:
	// Target track config accessing
	bool                        GetTargetTrackConfig(u32 uNameHash, TargetTrackHelpers::STargetTrackConfig const*& pOutConfig) const;
	bool                        GetTargetTrackStimulusConfig(u32 uNameHash, u32 uStimulusHash, TargetTrackHelpers::STargetTrackStimulusConfig const*& pOutConfig) const;
	const ITargetTrackModifier* GetTargetTrackModifier(u32 uId) const;

	class CTargetTrackConfigProxy : public TargetTrackHelpers::ITargetTrackConfigProxy
	{
	public:
		CTargetTrackConfigProxy(CTargetTrackUpr* pUpr) : m_pUpr(pUpr)
		{
			assert(m_pUpr);
		}

		bool GetTargetTrackConfig(u32 uNameHash, TargetTrackHelpers::STargetTrackConfig const*& pOutConfig) const
		{
			return m_pUpr->GetTargetTrackConfig(uNameHash, pOutConfig);
		}

		bool GetTargetTrackStimulusConfig(u32 uNameHash, u32 uStimulusHash, TargetTrackHelpers::STargetTrackStimulusConfig const*& pOutConfig) const
		{
			return m_pUpr->GetTargetTrackStimulusConfig(uNameHash, uStimulusHash, pOutConfig);
		}

		const ITargetTrackModifier* GetTargetTrackModifier(u32 uId) const
		{
			return m_pUpr->GetTargetTrackModifier(uId);
		}

		void ModifyTargetThreat(IAIObject& ownerAI, IAIObject& targetAI, const ITargetTrack& track, float& outThreatRatio, EAITargetThreat& outThreat) const;

	private:
		CTargetTrackUpr* m_pUpr;
	};

private:
	// Config load helpers
	bool LoadConfigs(XmlNodeRef& pRoot);
	bool LoadConfigStimuli(TargetTrackHelpers::STargetTrackConfig* pConfig, XmlNodeRef& pStimuliElement, bool bHasTemplate);
	bool LoadConfigModifiers(TargetTrackHelpers::STargetTrackStimulusConfig* pStimulusConfig, XmlNodeRef& pModifiersElement);
	bool LoadConfigPulses(TargetTrackHelpers::STargetTrackStimulusConfig* pStimulusConfig, XmlNodeRef& pPulsesElement);
	bool LoadConfigReleaseThreatLevels(TargetTrackHelpers::STargetTrackStimulusConfig* pStimulusConfig, XmlNodeRef& pReleaseThreatLevelsElement);
	bool ApplyStimulusTemplates();
	bool ApplyStimulusTemplate(TargetTrackHelpers::STargetTrackConfig* pConfig, const TargetTrackHelpers::STargetTrackConfig* pParent);

	// Serialize helpers
	void Serialize_Write(TSerialize ser);
	void Serialize_Read(TSerialize ser);
	bool RegisterAgent(tAIObjectID aiObjectId, u32 uConfigHash, i32 nTargetLimit = 0);

	// Stimulus helpers
	static tAIObjectID GetAIObjectId(EntityId entityId);
	bool               ShouldStimulusBeHandled(tAIObjectID aiObjectID, const TargetTrackHelpers::SStimulusEvent& stimulusEvent, const float maxRadius = FLT_MAX);
	bool               HandleStimulusEvent(CTargetTrackGroup* pGroup, TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent);
	bool               CheckConfigUsesStimulus(u32 uConfigHash, u32 uStimulusNameHash) const;
	bool               CheckStimulusHostile(tAIObjectID aiObjectId, tAIObjectID aiTargetId, u32 uConfigHash, u32 uStimulusNameHash) const;
	bool               TranslateVisualStimulusIfCanBeHandled(TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, const SAIEVENT* pAIEvent) const;
	bool               TranslateSoundStimulusIfCanBeHandled(TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, const SAIEVENT* pAIEvent) const;
	bool               TranslateBulletRainStimulusIfCanBeHandled(TargetTrackHelpers::STargetTrackStimulusEvent& stimulusEvent, const SAIEVENT* pAIEvent) const;

	static u32      GetConfigNameHash(tukk sName);
	static u32      GetStimulusNameHash(tukk sStimulus);
	static u32      GetPulseNameHash(tukk sPulse);

	void               PrepareModifiers();
	void               DeleteConfigs();
	void               DeleteAgents();
	void               ResetFreshestTargetData();

	typedef std::map<u32, TargetTrackHelpers::STargetTrackConfig*> TConfigContainer;
	TConfigContainer m_Configs;

	typedef std::map<tAIObjectID, CTargetTrackGroup*> TAgentContainer;
	TAgentContainer m_Agents;

	typedef std::vector<CTargetTrack*> TTargetTrackPoolContainer;
	TTargetTrackPoolContainer m_TargetTrackPool;

	typedef std::vector<ITargetTrackModifier*> TModifierContainer;
	TModifierContainer m_Modifiers;

	typedef std::map<tAIObjectID, float> TClassThreatContainer;
	TClassThreatContainer       m_ClassThreatValues;

	ITargetTrackThreatModifier* m_pThreatModifier;

	CTargetTrackPoolProxy*      m_pTrackPoolProxy;
	CTargetTrackConfigProxy*    m_pTrackConfigProxy;

	struct FreshData
	{
		FreshData()
			: timeOfFreshestVisualStimulus(0.0f)
			, freshestVisualPosition(ZERO)
			, freshestVisualDirection(ZERO)
		{
		}

		float timeOfFreshestVisualStimulus;
		Vec3  freshestVisualPosition;
		Vec3  freshestVisualDirection;
	};

	typedef tAIObjectID                            TargetAIObjectID;
	typedef VectorMap<TargetAIObjectID, FreshData> DataPerTarget;
	DataPerTarget m_dataPerTarget;

#ifdef TARGET_TRACK_DEBUG
	void DebugDrawConfig(i32 nMode);
	void DebugDrawTargets(i32 nMode, char const* szAgentName);
	void DebugDrawAgent(char const* szAgentName);

	// Used to give the agent being debugged one last frame to clean up when debug drawing is turned off for him
	tAIObjectID m_uLastDebugAgent;
#endif //TARGET_TRACK_DEBUG
};

#endif //__TARGET_TRACK_MANAGER_H__
