// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/ConditionsCollection.h>
#include <drx3D/DynRespSys/VariableCollection.h>
#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/SpecialConditionsImpl.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/ResponseUpr.h>

using namespace DrxDRS;

#if !defined(_RELEASE)
DRS::IResponseActor* CConditionsCollection::s_pActorForEvaluation = nullptr;
#endif

//--------------------------------------------------------------------------------------------------
bool CConditionsCollection::IsMet(CResponseInstance* pResponseInstance)
{
	for (SConditionInfo& current : m_conditions)
	{
		DRX_ASSERT(current.m_pCondition);

		if (!current.m_bNegated)
		{
			if (!current.m_pCondition->IsMet(pResponseInstance))
			{
				DRS_DEBUG_DATA_ACTION(AddConditionChecked(&current, m_bNegated));
				return m_bNegated;
			}
		}
		else
		{
			if (current.m_pCondition->IsMet(pResponseInstance))
			{
				DRS_DEBUG_DATA_ACTION(AddConditionChecked(&current, m_bNegated));
				return m_bNegated;
			}
		}
		DRS_DEBUG_DATA_ACTION(AddConditionChecked(&current, !m_bNegated));
	}

	return !m_bNegated;
}

//--------------------------------------------------------------------------------------------------
CConditionsCollection::~CConditionsCollection()
{
	m_conditions.clear();
}

//--------------------------------------------------------------------------------------------------
void CConditionsCollection::AddCondition(DRS::IConditionSharedPtr pCondition, bool Negated)
{
	if (pCondition)
	{
		SConditionInfo newCondition;
		newCondition.m_bNegated = Negated;
		newCondition.m_pCondition = pCondition;
		m_conditions.push_back(newCondition);
	}
}

//--------------------------------------------------------------------------------------------------
void CConditionsCollection::SConditionInfo::Serialize(Serialization::IArchive& ar)
{
	if (ar.isInput())
		m_pCondition = nullptr;

	ar(m_pCondition, "Condition", ">+^");

	if (!m_pCondition)
	{
		m_pCondition = DRS::IConditionSharedPtr(new CPlaceholderCondition());

		if (ar.isOutput())
		{
			ar(m_pCondition, "Condition", ">+^");
		}
	}
	ar(m_bNegated, "Negated", "^>Negated");

#if !defined(_RELEASE)
	if (ar.getFilter() & CResponseUpr::eSH_EvaluateResponses)
	{
		bool bCurrentlyMet = false;
		if (s_pActorForEvaluation)
		{
			SSignal tempSignal(CHashedString::GetEmpty(), static_cast<CResponseActor*>(s_pActorForEvaluation), nullptr);
			CResponseInstance tempResponseInstance(tempSignal, nullptr);
			bCurrentlyMet = m_pCondition->IsMet(&tempResponseInstance);
		}
		ar(bCurrentlyMet, "currentlyMet", "^^Currently met");
	}
#endif
}

//--------------------------------------------------------------------------------------------------
void CConditionsCollection::Serialize(Serialization::IArchive& ar)
{
	if (ar.openBlock("Conditions", "Conditions"))
	{
		ar(m_conditions, "Conditions", "^+");
		if (m_conditions.empty())
		{
			ar(m_bNegated, "Negated", "");
		}
		else
		{
			ar(m_bNegated, "Negated", "^Negated as whole");
		}

		ar.closeBlock();
	}
}
