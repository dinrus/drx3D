// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/ResponseUpr.h>
#include <drx3D/DynRespSys/VariableCollection.h>
#include <drx3D/DynRespSys/ActionSendSignal.h>
#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>

using namespace DrxDRS;

//--------------------------------------------------------------------------------------------------
DRS::IResponseActionInstanceUniquePtr CActionSendSignal::Execute(DRS::IResponseInstance* pResponseInstance)
{
	pResponseInstance->GetCurrentActor()->QueueSignal(m_signalName, (m_bCopyContextVariables) ? pResponseInstance->GetContextVariables() : nullptr);

	return nullptr;
}

//--------------------------------------------------------------------------------------------------
void CActionSendSignal::Serialize(Serialization::IArchive& ar)
{
	ar(m_signalName, "signal", "^ Signal");
#if defined(HASHEDSTRING_STORES_SOURCE_STRING)
	if (ar.isEdit())
	{
		if (!m_signalName.IsValid())
		{
			ar.warning(m_signalName.m_textCopy, "No signal to send specified");
		}
		else if (CResponseSystem::GetInstance()->GetResponseUpr()->GetResponse(m_signalName) == nullptr)
		{
			ar.warning(m_signalName.m_textCopy, "No response exists for the specified signal");
		}
	}
#endif
	ar(m_bCopyContextVariables, "copyContextVar", "^Copy Context Variable");
}

