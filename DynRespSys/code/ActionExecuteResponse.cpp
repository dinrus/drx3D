// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/ActionExecuteResponse.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/ResponseUpr.h>

using namespace DrxDRS;

DRS::IResponseActionInstanceUniquePtr CActionExecuteResponse::Execute(DRS::IResponseInstance* pResponseInstance)
{
	if (m_responseID.IsValid())
	{
		CActionExecuteResponseInstance* pInstance = new CActionExecuteResponseInstance();
		DRS::SignalInstanceId id = pResponseInstance->GetCurrentActor()->QueueSignal(m_responseID, pResponseInstance->GetContextVariables(), pInstance);
		return DRS::IResponseActionInstanceUniquePtr(pInstance);
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
void CActionExecuteResponse::Serialize(Serialization::IArchive& ar)
{
	ar(m_responseID, "response", "^ Response");

#if defined(HASHEDSTRING_STORES_SOURCE_STRING)
	if (ar.isEdit())
	{
		if (!m_responseID.IsValid())
		{
			ar.warning(m_responseID.m_textCopy, "No response to execute specified.");
		}
		else if (!CResponseSystem::GetInstance()->GetResponseUpr()->HasMappingForSignal(m_responseID))
		{
			ar.warning(m_responseID.m_textCopy, "Response to execute not found.");
		}
	}
#endif
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
CActionExecuteResponseInstance::~CActionExecuteResponseInstance()
{
	CResponseSystem::GetInstance()->GetResponseUpr()->RemoveListener(this);
}

//--------------------------------------------------------------------------------------------------
DRS::IResponseActionInstance::eCurrentState CActionExecuteResponseInstance::Update()
{
	return m_state;
}

//--------------------------------------------------------------------------------------------------
void CActionExecuteResponseInstance::Cancel()
{
	if (m_state == CS_RUNNING)
	{
		if (m_pStartedResponse)
		{
			m_pStartedResponse->Cancel();
		}
		m_state = CS_CANCELED;
	}
}

//--------------------------------------------------------------------------------------------------
void CActionExecuteResponseInstance::OnSignalProcessingFinished(SSignalInfos& signal, DRS::IResponseInstance* pFinishedResponse, eProcessingResult outcome)
{
	m_state = CS_FINISHED;
}

//--------------------------------------------------------------------------------------------------
void DrxDRS::CActionExecuteResponseInstance::OnSignalProcessingStarted(SSignalInfos& signal, DRS::IResponseInstance* pStartedResponse)
{
	m_pStartedResponse = static_cast<CResponseInstance*>(pStartedResponse);
}
