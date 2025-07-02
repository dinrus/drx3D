// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/HashedString.h>
#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

struct ICVar;

namespace DrxDRS
{
class CVariable;
class CResponseActor;
class CDialogLine;

class CDefaultLipsyncProvider final : public DRS::ISpeakerUpr::ILipsyncProvider
{
	//for now, our default lipsync provider just starts a animation when a line starts, and stops it when the line finishes
public:
	CDefaultLipsyncProvider();
	virtual ~CDefaultLipsyncProvider() override;

	//////////////////////////////////////////////////////////////////////////
	// ISpeakerUpr::ILipsyncProvider implementation
	virtual DRS::LipSyncID OnLineStarted(DRS::IResponseActor* pSpeaker, const DRS::IDialogLine* pLine) override;
	virtual void           OnLineEnded(DRS::LipSyncID lipsyncId, DRS::IResponseActor* pSpeaker, const DRS::IDialogLine* pLine) override;
	virtual bool           Update(DRS::LipSyncID lipsyncId, DRS::IResponseActor* pSpeaker, const DRS::IDialogLine* pLine) override;
	//////////////////////////////////////////////////////////////////////////

protected:
	i32    m_lipsyncAnimationLayer;
	float  m_lipsyncTransitionTime;
	string m_defaultLipsyncAnimationName;
};

class CSpeakerUpr final : public DRS::ISpeakerUpr
{
public:
	CSpeakerUpr();
	virtual ~CSpeakerUpr() override;

	void Init();
	void Shutdown();
	void Reset();
	void Update();

	void OnActorRemoved(const CResponseActor* pActor);

	//////////////////////////////////////////////////////////////////////////
	// ISpeakerUpr implementation
	virtual bool                  IsSpeaking(const DRS::IResponseActor* pActor, const CHashedString& lineID = CHashedString::GetEmpty(), bool bCheckQueuedLinesAsWell = false) const override;
	virtual IListener::eLineEvent StartSpeaking(DRS::IResponseActor* pActor, const CHashedString& lineID) override;
	virtual bool                  CancelSpeaking(const DRS::IResponseActor* pActor, i32 maxPrioToCancel = -1, const CHashedString& lineID = CHashedString::GetEmpty(), bool bCancelQueuedLines = true) override;
	virtual bool                  AddListener(DRS::ISpeakerUpr::IListener* pListener) override;
	virtual bool                  RemoveListener(DRS::ISpeakerUpr::IListener* pListener) override;
	virtual void                  SetCustomLipsyncProvider(DRS::ISpeakerUpr::ILipsyncProvider* pProvider) override;
	//////////////////////////////////////////////////////////////////////////

private:
	enum EEndingConditions
	{
		eEC_Done                   = 0,
		eEC_WaitingForStartTrigger = BIT(0),
		eEC_WaitingForStopTrigger  = BIT(1),
		eEC_WaitingForTimer        = BIT(2),
		eEC_WaitingForLipsync      = BIT(3)
	};

	struct SSpeakInfo
	{
		SSpeakInfo() = default;
		SSpeakInfo(DrxAudio::AuxObjectId auxAudioObjectId) : speechAuxObjectId(auxAudioObjectId), voiceAttachmentIndex(-1) {}

		CResponseActor*       pActor;
		IEntity*              pEntity;
		string                text;
		CHashedString         lineID;
		const CDialogLine*    pPickedLine;
		float                 finishTime;
		i32                   priority;

		i32                   voiceAttachmentIndex;      //cached index of the voice attachment index // -1 means invalid ID;
		DrxAudio::AuxObjectId speechAuxObjectId;
		DrxAudio::ControlId   startTriggerID;
		DrxAudio::ControlId   stopTriggerID;
		string                standaloneFile;

		u32                endingConditions;      //EEndingConditions
		DRS::LipSyncID        lipsyncId;
		bool                  bWasCanceled;
	};

	struct SWaitingInfo
	{
		CResponseActor* pActor;
		CHashedString   lineID;
		i32             linePriority;
		float           waitEndTime;

		bool operator==(const SWaitingInfo& other) const { return pActor == other.pActor && lineID == other.lineID; }  //no need to check prio or endTime, since we only allow one instance of the same lineId per speaker
	};

	void        UpdateAudioProxyPosition(IEntity* pEntity, const SSpeakInfo& newSpeakerInfo);
	void        ReleaseSpeakerAudioProxy(SSpeakInfo& speakerInfo, bool stopTrigger);
	static void OnAudioCallback(DrxAudio::SRequestInfo const* const pAudioRequestInfo);

	void        InformListener(const DRS::IResponseActor* pSpeaker, const CHashedString& lineID, DRS::ISpeakerUpr::IListener::eLineEvent event, const CDialogLine* pLine);
	bool        OnLineAboutToStart(const DRS::IResponseActor* pSpeaker, const CHashedString& lineID);
	void        SetNumActiveSpeaker(i32 newAmountOfSpeaker);

	//the pure execution, without further checking if the line can be started (that should have happen before)
	void ExecuteStartSpeaking(SSpeakInfo* pSpeakerInfoToUse);

	void QueueLine(CResponseActor* pActor, const CHashedString& lineID, float maxQueueDuration, i32k priority);

	typedef std::vector<SSpeakInfo> SpeakerList;
	SpeakerList m_activeSpeakers;

	typedef std::vector<SWaitingInfo> QueuedSpeakerList;
	QueuedSpeakerList m_queuedSpeakers;

	typedef CListenerSet<DRS::ISpeakerUpr::IListener*> ListenerList;
	ListenerList                            m_listeners;

	DRS::ISpeakerUpr::ILipsyncProvider* m_pLipsyncProvider;
	CDefaultLipsyncProvider*                m_pDefaultLipsyncProvider;

	i32                                            m_numActiveSpeaker;
	DrxAudio::ControlId                            m_audioParameterIdLocal;
	DrxAudio::ControlId                            m_audioParameterIdGlobal;

	std::vector<std::pair<CResponseActor*, float>> m_recentlyFinishedSpeakers;

	// CVars
	i32          m_displaySubtitlesCVar;
	i32          m_playAudioCVar;
	i32          m_samePrioCancelsLinesCVar;
	float        m_defaultMaxQueueTime;
	static float s_defaultPauseAfterLines;
	ICVar*       m_pDrsDialogDialogRunningEntityParameterName;
	ICVar*       m_pDrsDialogDialogRunningGlobalParameterName;
};
}  //namespace DrxDRS
