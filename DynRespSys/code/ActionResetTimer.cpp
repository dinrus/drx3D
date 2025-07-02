// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include <drx3D/DynRespSys/ActionResetTimer.h>
#include <drx3D/DynRespSys/VariableCollection.h>
#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/ResponseInstance.h>

using namespace DrxDRS;

//////////////////////////////////////////////////////////
DRS::IResponseActionInstanceUniquePtr CActionResetTimerVariable::Execute(DRS::IResponseInstance* pResponseInstance)
{
	CVariableCollection* pCollectionToUse = GetCurrentCollection(static_cast<CResponseInstance*>(pResponseInstance));
	if (!pCollectionToUse)
	{
		CVariableCollectionUpr* pVcUpr = CResponseSystem::GetInstance()->GetVariableCollectionUpr();
		pCollectionToUse = pVcUpr->CreateVariableCollection(m_collectionName);
	}
	if (pCollectionToUse)
	{
		pCollectionToUse->SetVariableValue(m_variableName, CResponseSystem::GetInstance()->GetCurrentDrsTime());
	}
	return nullptr;
}

//////////////////////////////////////////////////////////
string CActionResetTimerVariable::GetVerboseInfo() const
{
	return "in variable" + GetVariableVerboseName();
}

//////////////////////////////////////////////////////////
void CActionResetTimerVariable::Serialize(Serialization::IArchive& ar)
{
	_Serialize(ar, "^TimerVariable");

#if defined (ENABLE_VARIABLE_VALUE_TYPE_CHECKINGS)
	CVariableCollection* pCollection = CResponseSystem::GetInstance()->GetCollection(m_collectionName);
	if (pCollection)
	{
		if (pCollection->GetVariableValue(m_variableName).GetType() != eDRVT_Float && pCollection->GetVariableValue(m_variableName).GetType() != eDRVT_Undefined)
		{
			ar.warning(m_collectionName.m_textCopy, "Variable to set needs to hold a time value as float");
		}
	}
#endif
}
