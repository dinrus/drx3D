// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowDialogNode.cpp
//  Version:     v1.00
//  Created:     14-07-2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DrxActionCVars.h>
#include <drx3D/Act/DinrusAction.h>

#include <drx3D/Act/DialogQueuesUpr.h>
#include <drx3D/Act/DialogSystem.h>
#include <drx3D/Act/DialogSession.h>

#include <drx3D/FlowGraph/IFlowBaseNode.h>

//HD-Remark: This is the Flow node for the legacy dialog system

#define DIALOG_NODE_FINAL
// #undef  DIALOG_NODE_FINAL
//////////////////////////////////////////////////////////////////////////

class CFlowDialogNode : public CFlowBaseNode<eNCT_Instanced>, public IDialogSessionListener
{
public:
	CFlowDialogNode(SActivationInfo* pActInfo)
	{
		m_sessionID = 0;
		m_bIsPlaying = false;
		m_actInfo = *pActInfo;
		m_buffer = CDialogQueuesUpr::NO_QUEUE;
		m_idForQueueUpr = 0;
		m_bufferDelayLeft = 0;
	}

	~CFlowDialogNode()
	{
		StopDialog();
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowDialogNode(pActInfo);
	}

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

	virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		ser.Value("m_sessionID", m_sessionID);
		ser.Value("m_buffer", m_buffer);
		ser.Value("m_idForBufferMgr", m_idForQueueUpr);
		ser.Value("m_bufferDelayLeft", m_bufferDelayLeft);
		if (ser.IsReading() && m_sessionID != 0)
		{
			CDialogSession* pSession = GetSession();
			assert(pSession != 0);
			if (pSession)
			{
				pSession->AddListener(this);
			}
		}
	}

	static i32k MAX_ACTORS = 8;

	enum
	{
		EIP_Play = 0,
		EIP_Stop,
		EIP_Dialog,
		EIP_StartLine,
		EIP_AIInterrupt,
		EIP_AwareDist,
		EIP_AwareAngle,
		EIP_AwareTimeOut,
		EIP_Flags,
		EIP_BufferId,
		EIP_BufferDelay,
		EIP_ActorFirst,
		EIP_ActorLast = EIP_ActorFirst + MAX_ACTORS - 1,
	};

	enum
	{
		EOP_Started = 0,
		EOP_DoneFinishedOrAborted,
		EOP_Finished,
		EOP_Aborted,
		EOP_PlayerAbort,
		EOP_AIAbort,
		EOP_ActorDied,
		EOP_LastLine,
		EOP_CurrentLine
	};

	void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Play",             _HELP("Trigger to play the Dialog")),
			InputPortConfig_Void("Stop",             _HELP("Trigger to stop the Dialog")),
			InputPortConfig<string>("dialog_Dialog", _HELP("Dialog to play"),             _HELP("Dialog")),
			InputPortConfig<i32>("StartLine",        0,                                   _HELP("Line to start Dialog from")),
			InputPortConfig<i32>("AIInterrupt",      0,                                   _HELP("AI interrupt behaviour: Never, Alert, Combat"),                                                                                                                                0,  _UICONFIG("enum_int:Alert=0,Combat=1,Never=2")),
			InputPortConfig<float>("AwareDistance",  0.f,                                 _HELP("Max. Distance Player is considered as listening. 0.0 disables check.")),
			InputPortConfig<float>("AwareAngle",     0.f,                                 _HELP("Max. View Angle Player is considered as listening. 0.0 disables check")),
			InputPortConfig<float>("AwareTimeout",   3.0f,                                _HELP("TimeOut until non-aware Player aborts dialog. [Effective pnly if AwareDistance or AwareAngle != 0]")),
			InputPortConfig<i32>("Flags",            0,                                   _HELP("Dialog Playback Flags"),                                                                                                                                                       0,  _UICONFIG("enum_int:None=0,FinishLineOnAbort=1")),
			InputPortConfig<i32>("Buffer",           0,                                   _HELP("Only 1 dialog can be played at any time in each buffer."),                                                                                                                     0,  _UICONFIG("enum_global:dialogBuffers")),
			InputPortConfig<float>("BufferDelay",    0,                                   _HELP("How much aditional time (seconds) this dialog will wait after the previous dialog in its buffer is done. (not used if buffer=NoBuffer, or if the buffer was empty already)")),
			InputPortConfig<EntityId>("Actor1",      _HELP("Actor 1 [EntityID]"),         _HELP("Actor 1")),
			InputPortConfig<EntityId>("Actor2",      _HELP("Actor 2 [EntityID]"),         _HELP("Actor 2")),
			InputPortConfig<EntityId>("Actor3",      _HELP("Actor 3 [EntityID]"),         _HELP("Actor 3")),
			InputPortConfig<EntityId>("Actor4",      _HELP("Actor 4 [EntityID]"),         _HELP("Actor 4")),
			InputPortConfig<EntityId>("Actor5",      _HELP("Actor 5 [EntityID]"),         _HELP("Actor 5")),
			InputPortConfig<EntityId>("Actor6",      _HELP("Actor 6 [EntityID]"),         _HELP("Actor 6")),
			InputPortConfig<EntityId>("Actor7",      _HELP("Actor 7 [EntityID]"),         _HELP("Actor 7")),
			InputPortConfig<EntityId>("Actor8",      _HELP("Actor 8 [EntityID]"),         _HELP("Actor 8")),
			{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_Void("Started",     _HELP("Triggered when the Dialog started.")),
			OutputPortConfig_Void("DoneFOA",     _HELP("Triggered when the Dialog ended [Finished OR Aborted]."),                                         _HELP("Done")),
			OutputPortConfig_Void("Done",        _HELP("Triggered when the Dialog finished normally."),                                                   _HELP("Finished")),
			OutputPortConfig_Void("Aborted",     _HELP("Triggered when the Dialog was aborted.")),
			OutputPortConfig<i32>("PlayerAbort", _HELP("Triggered when the Dialog was aborted because Player was not aware.\n1=OutOfRange\n2=OutOfView")),
			OutputPortConfig_Void("AIAbort",     _HELP("Triggered when the Dialog was aborted because AI got alerted.")),
			OutputPortConfig_Void("ActorDied",   _HELP("Triggered when the Dialog was aborted because an Actor died.")),
			OutputPortConfig<i32>("LastLine",    _HELP("Last line when dialog was aborted.")),
			OutputPortConfig<i32>("CurLine",     _HELP("Current line. Triggered whenever a line starts.")),
			{ 0 }
		};
		config.sDescription = _HELP("Play a Dialog - WIP");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.nFlags |= EFLN_AISEQUENCE_SUPPORTED;
		config.SetCategory(EFLN_ADVANCED);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
			break;
		case eFE_Activate:
			if (IsPortActive(pActInfo, EIP_Stop))
			{
				bool bOk = StopDialog();
				if (bOk)
					ActivateOutput(pActInfo, EOP_DoneFinishedOrAborted, true);
			}
			if (IsPortActive(pActInfo, EIP_Play))
			{
				bool bOk = StopDialog();
				if (bOk)
					ActivateOutput(pActInfo, EOP_DoneFinishedOrAborted, true);

				const string& dialogString = GetPortString(pActInfo, EIP_Dialog);
				CDialogQueuesUpr* pMgr = CDrxAction::GetDrxAction()->GetDialogSystem()->GetDialogQueueUpr();
				m_buffer = pMgr->BufferEnumToId(GetPortInt(pActInfo, EIP_BufferId)); // we get the buffer value at this time and use it from now on. If something changes the port value after that, we are not going to care! at all!
				if (pMgr->IsBufferFree(m_buffer))
				{
					bOk = PlayDialog(pActInfo);
					if (bOk)
						m_idForQueueUpr = pMgr->Play(m_buffer, dialogString);
				}
				else
				{
					m_idForQueueUpr = pMgr->Play(m_buffer, dialogString);
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
					m_bufferDelayLeft = GetPortFloat(pActInfo, EIP_BufferDelay);
				}
			}
			break;

		case eFE_Update:
			{
				if (m_buffer == CDialogQueuesUpr::NO_QUEUE) // if is updating, and yet nobuffer, only can be because it has been stopped abruptly
				{
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
				}
				else
				{
					if (CDialogSystem* pDS = CDrxAction::GetDrxAction()->GetDialogSystem())
					{
						CDialogQueuesUpr* pMgr = pDS->GetDialogQueueUpr();

						if (!pMgr->IsDialogWaiting(m_buffer, m_idForQueueUpr))
						{
							if (m_bufferDelayLeft > 0)
								m_bufferDelayLeft -= gEnv->pTimer->GetFrameTime();
							else
							{
								pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
								bool bOk = PlayDialog(pActInfo);
								if (!bOk)
									NotifyToBufferMgrDialogStopped();
							}
						}
					}
				}
				break;
			}
		}
	}

protected:

	void NotifyToBufferMgrDialogStopped()
	{
		if (m_buffer != CDialogQueuesUpr::NO_QUEUE)
		{
			if (CDialogSystem* pDS = CDrxAction::GetDrxAction()->GetDialogSystem())
			{
				CDialogQueuesUpr* pMgr = pDS->GetDialogQueueUpr();
				pMgr->NotifyDialogDone(m_buffer, m_idForQueueUpr);
				m_buffer = CDialogQueuesUpr::NO_QUEUE;
			}
		}
	}

	CDialogSession* GetSession()
	{
		CDialogSystem* pDS = NULL;
		CDrxAction* pDrxAction = CDrxAction::GetDrxAction();
		if (pDrxAction)
			pDS = pDrxAction->GetDialogSystem();
		if (pDS)
			return pDS->GetSession(m_sessionID);
		return 0;
	}

	bool StopDialog()
	{
		NotifyToBufferMgrDialogStopped();
		m_bIsPlaying = false;
		CDialogSession* pSession = GetSession();
		if (pSession)
		{
			// we always remove first, so we don't get notified
			pSession->RemoveListener(this);

			CDialogSystem* pDS = CDrxAction::GetDrxAction()->GetDialogSystem();
			if (pDS)
			{
				pDS->DeleteSession(m_sessionID);
			}
			m_sessionID = 0;
			return true;
		}
		return false;
	}

	bool PlayDialog(SActivationInfo* pActInfo)
	{
		CDialogSystem* pDS = CDrxAction::GetDrxAction()->GetDialogSystem();
		if (!pDS)
			return true;
		i32k fromLine = GetPortInt(pActInfo, EIP_StartLine);
		const string& scriptID = GetPortString(pActInfo, EIP_Dialog);
		m_sessionID = pDS->CreateSession(scriptID);
		if (m_sessionID == 0)
		{
			GameWarning("[flow] PlayDialog: Cannot create DialogSession with Script '%s'.", scriptID.c_str());
			return false;
		}

		CDialogSession* pSession = GetSession();
		assert(pSession != 0);
		if (pSession == 0)
			return false;

		// set actor flags (actually we could have flags per actor, but we use the same Flags for all of them)
		CDialogSession::TActorFlags actorFlags = CDialogSession::eDACF_Default;
		switch (GetPortInt(pActInfo, EIP_Flags))
		{
		case 1:
			actorFlags = CDialogSession::eDACF_NoAbortSound;
			break;
		default:
			break;
		}

		// stage actors
		for (i32 i = 0; i < MAX_ACTORS; ++i)
		{
			EntityId id = GetPortEntityId(pActInfo, i + EIP_ActorFirst);
			if (id != 0)
			{
				pSession->SetActor(static_cast<CDialogScript::TActorID>(i), id);
				pSession->SetActorFlags(static_cast<CDialogScript::TActorID>(i), actorFlags);
			}
		}

		pSession->SetAIBehaviourMode(CDialogSession::eDIB_InterruptNever);
		pSession->SetPlayerAwarenessDistance(GetPortFloat(pActInfo, EIP_AwareDist));
		pSession->SetPlayerAwarenessAngle(GetPortFloat(pActInfo, EIP_AwareAngle));
		pSession->SetPlayerAwarenessGraceTime(GetPortFloat(pActInfo, EIP_AwareTimeOut));

		i32k alertnessInterruptModeInput = GetPortInt(pActInfo, EIP_AIInterrupt);
		CDialogSession::AlertnessInterruptMode alertnessInterruptMode = CDialogSession::None;
		switch (alertnessInterruptModeInput)
		{
		case 0:
			alertnessInterruptMode = CDialogSession::Alert;
			break;
		case 1:
			alertnessInterruptMode = CDialogSession::Combat;
			break;
		case 2:
			alertnessInterruptMode = CDialogSession::None;
			break;
		default:
			assert(false);
			break;
		}
		pSession->SetAlertnessInterruptMode(alertnessInterruptMode);

		// Validate the session
		if (pSession->Validate() == false)
		{
			CDialogScript::SActorSet currentSet = pSession->GetCurrentActorSet();
			CDialogScript::SActorSet reqSet = pSession->GetScript()->GetRequiredActorSet();
			GameWarning("[flow] PlayDialog: Session with Script '%s' cannot be validated: ", scriptID.c_str());
			for (i32 i = 0; i < CDialogScript::MAX_ACTORS; ++i)
			{
				if (reqSet.HasActor(i) && !currentSet.HasActor(i))
				{
					GameWarning("[flow]  Actor %d is missing.", i + 1);
				}
			}

			pDS->DeleteSession(m_sessionID);
			m_sessionID = 0;
			return false;
		}

		pSession->AddListener(this);

		const bool bPlaying = pSession->Play(fromLine);
		if (!bPlaying)
		{
			pSession->RemoveListener(this);
			pDS->DeleteSession(m_sessionID);
			m_sessionID = 0;
		}

		return m_sessionID != 0;
	}
protected:
	// IDialogSessionListener
	virtual void SessionEvent(CDialogSession* pSession, CDialogSession::EDialogSessionEvent event)
	{
		if (pSession && pSession->GetSessionID() == m_sessionID)
		{
			switch (event)
			{
			case CDialogSession::eDSE_SessionStart:
				ActivateOutput(&m_actInfo, EOP_Started, true);
				m_bIsPlaying = true;
				break;
			case CDialogSession::eDSE_Aborted:
				{
					const CDialogSession::EAbortReason reason = pSession->GetAbortReason();
					i32k curLine = pSession->GetCurrentLine();
					StopDialog();
					ActivateOutput(&m_actInfo, EOP_DoneFinishedOrAborted, true);
					ActivateOutput(&m_actInfo, EOP_Aborted, true);
					ActivateOutput(&m_actInfo, EOP_LastLine, curLine);

					if (reason == CDialogSession::eAR_AIAborted)
						ActivateOutput(&m_actInfo, EOP_AIAbort, true);
					else if (reason == CDialogSession::eAR_PlayerOutOfRange)
						ActivateOutput(&m_actInfo, EOP_PlayerAbort, 1);
					else if (reason == CDialogSession::eAR_PlayerOutOfView)
						ActivateOutput(&m_actInfo, EOP_PlayerAbort, 2);
					else if (reason == CDialogSession::eAR_ActorDead)
						ActivateOutput(&m_actInfo, EOP_ActorDied, true);
				}
				break;
			case CDialogSession::eDSE_EndOfDialog:
				StopDialog();
				ActivateOutput(&m_actInfo, EOP_Finished, true);
				ActivateOutput(&m_actInfo, EOP_DoneFinishedOrAborted, true);
				break;
			case CDialogSession::eDSE_UserStopped:
				StopDialog();
				ActivateOutput(&m_actInfo, EOP_DoneFinishedOrAborted, true);
				break;
			case CDialogSession::eDSE_SessionDeleted:
				m_sessionID = 0;
				m_bIsPlaying = false;
				NotifyToBufferMgrDialogStopped();
				break;
			case CDialogSession::eDSE_LineStarted:
				ActivateOutput(&m_actInfo, EOP_CurrentLine, pSession->GetCurrentLine());
				break;
			}
		}
	}
	// ~IDialogSessionListener

private:
	SActivationInfo                 m_actInfo;
	CDialogSystem::SessionID        m_sessionID;
	CDialogQueuesUpr::TDialogId m_idForQueueUpr;
	u32                          m_buffer;
	float                           m_bufferDelayLeft;
	bool                            m_bIsPlaying;
};

#ifdef DIALOG_NODE_FINAL
REGISTER_FLOW_NODE("Dialog:PlayDialog", CFlowDialogNode);
#endif
