// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ActionExecuteAudioTrigger.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>

static tukk ActionPlaySoundId = "ExecuteAudioTriggerAction";

DRS::IResponseActionInstanceUniquePtr CActionExecuteAudioTrigger::Execute(DRS::IResponseInstance* pResponseInstance)
{
	AudioControlId audioStartTriggerID = INVALID_AUDIO_CONTROL_ID;
	if (gEnv->pAudioSystem->GetAudioTriggerId(m_AudioTriggerName.c_str(), audioStartTriggerID))
	{
		IEntity* pEntity = pResponseInstance->GetCurrentActor()->GetLinkedEntity();
		if (pEntity)
		{
			const IEntityAudioProxyPtr pEntityAudioProxy = crycomponent_cast<IEntityAudioProxyPtr>(pEntity->CreateProxy(ENTITY_PROXY_AUDIO));

			DRS::IResponseActionInstanceUniquePtr pActionInstance(new CActionExecuteAudioTriggerInstance());
			
			if (m_bWaitToBeFinished)
			{
				SAudioCallBackInfo const callbackInfo((uk const)pActionInstance.get(), (uk const)ActionPlaySoundId, (uk const)pActionInstance.get(), eAudioRequestFlags_PriorityNormal | eAudioRequestFlags_SyncFinishedCallback);
				if (pEntityAudioProxy->ExecuteTrigger(audioStartTriggerID, DEFAULT_AUDIO_PROXY_ID, callbackInfo))
				{
					return pActionInstance;
				}
			}
			else
			{
				pEntityAudioProxy->ExecuteTrigger(audioStartTriggerID, DEFAULT_AUDIO_PROXY_ID);
			}
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------
string CActionExecuteAudioTrigger::GetVerboseInfo() const
{
	return string("'") + m_AudioTriggerName + "'";
}

//--------------------------------------------------------------------------------------------------
void CActionExecuteAudioTrigger::Serialize(Serialization::IArchive& ar)
{
	ar(m_AudioTriggerName, "AudioTriggerName", "^ TriggerName");
	ar(m_bWaitToBeFinished, "WaitForFinish", "^ WaitForTriggerToFinish");
}

//--------------------------------------------------------------------------------------------------
CActionExecuteAudioTriggerInstance::CActionExecuteAudioTriggerInstance() : m_bHasFinished(false)
{
	gEnv->pAudioSystem->AddRequestListener(&CActionExecuteAudioTriggerInstance::OnAudioTriggerFinished, this, eAudioRequestType_AudioCallbackUprRequest, eAudioCallbackUprRequestType_ReportFinishedTriggerInstance);
	gEnv->pAudioSystem->AddRequestListener(&CActionExecuteAudioTriggerInstance::OnAudioTriggerFinished, this, eAudioRequestType_AudioObjectRequest, eAudioObjectRequestType_ExecuteTrigger);
}

//--------------------------------------------------------------------------------------------------
CActionExecuteAudioTriggerInstance::~CActionExecuteAudioTriggerInstance()
{
	gEnv->pAudioSystem->RemoveRequestListener(&CActionExecuteAudioTriggerInstance::OnAudioTriggerFinished, this);
	gEnv->pAudioSystem->RemoveRequestListener(&CActionExecuteAudioTriggerInstance::OnAudioTriggerFinished, this);
}

//--------------------------------------------------------------------------------------------------
DRS::IResponseActionInstance::eCurrentState CActionExecuteAudioTriggerInstance::Update()
{
	if (!m_bHasFinished) 
		return DRS::IResponseActionInstance::CS_RUNNING;
	else 
		return DRS::IResponseActionInstance::CS_FINISHED;
}

//--------------------------------------------------------------------------------------------------
void CActionExecuteAudioTriggerInstance::OnAudioTriggerFinished(const SAudioRequestInfo* const pInfo)
{
	if (pInfo->pUserData == ActionPlaySoundId)
	{
		if ((pInfo->audioRequestType == eAudioRequestType_AudioObjectRequest && pInfo->specificAudioRequest == eAudioObjectRequestType_ExecuteTrigger && pInfo->requestResult == eAudioRequestResult_Failure)
			|| (pInfo->audioRequestType == eAudioRequestType_AudioCallbackUprRequest && pInfo->specificAudioRequest == eAudioCallbackUprRequestType_ReportFinishedTriggerInstance))
		{
			DRX_ASSERT(pInfo->pUserDataOwner);
			CActionExecuteAudioTriggerInstance* pEndedInstance = reinterpret_cast<CActionExecuteAudioTriggerInstance*>(pInfo->pUserDataOwner);
			pEndedInstance->SetFinished(true);
		}
	}
}
