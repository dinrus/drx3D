// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/HashedString.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>

namespace DrxDRS
{
class CResponseActor;
class CResponseInstance;

class CActionSpeakLine final : public DRS::IResponseAction
{
public:
	enum ESpeakLineFlags
	{
		eSpeakLineFlags_None                          = 0,
		eSpeakLineFlags_CancelResponseOnSkip          = BIT(1),
		eSpeakLineFlags_CancelResponseOnCanceled      = BIT(2),
		ESpeakLineFlags_SendSignalOnStart             = BIT(3),
		ESpeakLineFlags_SendSignalOnSkip              = BIT(4),
		ESpeakLineFlags_SendSignalOnCancel            = BIT(5),
		ESpeakLineFlags_SendSignalOnFinish            = BIT(7),
		ESpeakLineFlags_ReevaluteConditionsAfterQueue = BIT(8),
		ESpeakLineFlags_Default                       = ESpeakLineFlags_ReevaluteConditionsAfterQueue | eSpeakLineFlags_CancelResponseOnSkip | eSpeakLineFlags_CancelResponseOnCanceled
	};

	CActionSpeakLine() : m_flags(ESpeakLineFlags_Default) {}
	CActionSpeakLine(const CHashedString& lineToSpeak) : m_lineIDToSpeak(lineToSpeak), m_flags(ESpeakLineFlags_Default) {}

	//////////////////////////////////////////////////////////
	// IResponseAction implementation
	virtual DRS::IResponseActionInstanceUniquePtr Execute(DRS::IResponseInstance* pResponseInstance) override;
	virtual string                                GetVerboseInfo() const override { return "'" + m_lineIDToSpeak.GetText() + "'"; }
	virtual void                                  Serialize(Serialization::IArchive& ar) override;
	virtual tukk                           GetType() const override        { return "Speak Line"; }
	//////////////////////////////////////////////////////////

private:
	CHashedString m_speakerOverrideName;
	CHashedString m_lineIDToSpeak;
	u32        m_flags;            //ESpeakLineFlags
};

//////////////////////////////////////////////////////////

class CActionSpeakLineInstance final : public DRS::IResponseActionInstance, DRS::ISpeakerUpr::IListener
{
public:
	CActionSpeakLineInstance(CResponseActor* pSpeaker, const CHashedString& lineID, CResponseInstance* pResponseInstance, u32 flags);
	virtual ~CActionSpeakLineInstance() override;

	//////////////////////////////////////////////////////////
	// IResponseActionInstance implementation
	virtual eCurrentState Update() override;
	virtual void          Cancel() override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// DRS::ISpeakerUpr::IListener implementation
	virtual bool OnLineAboutToStart(const DRS::IResponseActor* pSpeaker, const CHashedString& lineID) override;
	virtual void OnLineEvent(const DRS::IResponseActor* pSpeaker, const CHashedString& lineID, DRS::ISpeakerUpr::IListener::eLineEvent lineEvent, const DRS::IDialogLine* pLine) override;
	//////////////////////////////////////////////////////////

private:
	CResponseActor* const    m_pSpeaker;
	const CHashedString      m_lineID;
	CResponseInstance* const m_pResponseInstance;

	eCurrentState            m_currentState;
	u32                   m_flags;
};

//////////////////////////////////////////////////////////

class CActionCancelSpeaking final : public DRS::IResponseAction
{
public:
	CActionCancelSpeaking();
	CActionCancelSpeaking(const CHashedString& speakerName);

	//////////////////////////////////////////////////////////
	// IResponseAction implementation
	virtual DRS::IResponseActionInstanceUniquePtr Execute(DRS::IResponseInstance* pResponseInstance) override;
	virtual string                                GetVerboseInfo() const override { return " for speaker '" + m_speakerOverrideName.GetText() + "'"; }
	virtual void                                  Serialize(Serialization::IArchive& ar) override;
	virtual tukk                           GetType() const override        { return "Cancel Line"; }
	//////////////////////////////////////////////////////////

private:
	CHashedString m_speakerOverrideName;
	i32           m_maxPrioToCancel;
	CHashedString m_lineId;
};

//////////////////////////////////////////////////////////

class CActionCancelSpeakingInstance final : public DRS::IResponseActionInstance
{
public:
	CActionCancelSpeakingInstance(const CResponseActor* pSpeaker, const CHashedString& lineId) : m_pSpeaker(pSpeaker), m_lineId(lineId) {}

	//////////////////////////////////////////////////////////
	// IResponseActionInstance implementation
	virtual eCurrentState Update() override;
	virtual void          Cancel() override;
	//////////////////////////////////////////////////////////

private:
	const CResponseActor* m_pSpeaker;
	const CHashedString   m_lineId;
};
}  //namespace DrxDRS
