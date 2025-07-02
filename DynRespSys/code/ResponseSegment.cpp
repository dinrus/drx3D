// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/ResponseSegment.h>

#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/CoreX/String/StringUtils.h>

#include <drx3D/DynRespSys/ActionSpeakLine.h>
#include <drx3D/DynRespSys/ConditionImpl.h>
#include <drx3D/DynRespSys/DelayActionAction.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/ResponseUpr.h>
#include <drx3D/DynRespSys/ResponseSystem.h>

using namespace DrxDRS;

//--------------------------------------------------------------------------------------------------
CResponseSegment::CResponseSegment() : m_pChildSegments(nullptr)
{
#if !defined(_RELEASE)
	m_name = "(AUTO) unnamed";
#endif
}

//--------------------------------------------------------------------------------------------------
CResponseSegment::CResponseSegment(CResponseSegment&& other)
	: m_responseActions(std::move(other.m_responseActions))
	, m_conditions(std::move(other.m_conditions))
	, m_pChildSegments(other.m_pChildSegments)
{
	other.m_pChildSegments = nullptr;
#if !defined(_RELEASE)
	m_name = std::move(other.m_name);
#endif
}

//--------------------------------------------------------------------------------------------------
CResponseSegment& CResponseSegment::operator=(CResponseSegment&& other)
{
	m_responseActions = std::move(other.m_responseActions);
	m_conditions = std::move(other.m_conditions);
	m_pChildSegments = other.m_pChildSegments;
	other.m_pChildSegments = nullptr;
#if !defined(_RELEASE)
	m_name = std::move(other.m_name);
#endif
	return *this;
}

//--------------------------------------------------------------------------------------------------
CResponseSegment::~CResponseSegment()
{
	delete m_pChildSegments;
}

//--------------------------------------------------------------------------------------------------
void CResponseSegment::AddCondition(DRS::IConditionSharedPtr pCondition, bool Negated)
{
	m_conditions.AddCondition(pCondition, Negated);
}

//--------------------------------------------------------------------------------------------------
void CResponseSegment::AddAction(DRS::IResponseActionSharedPtr pAction)
{
	ActionsInfo newAction;
	newAction.m_pAction = pAction;
	m_responseActions.push_back(newAction);
}

//--------------------------------------------------------------------------------------------------
CResponseSegment* CResponseSegment::GetNextResponseSegment(CResponseInstance* pResponseInstance)
{
	if (!m_pChildSegments)
	{
		return nullptr;
	}

	u32 bestMatchedConditionsCountYet = 0;  //we want to start the responseSegment with the most matching conditions
	static std::vector<CResponseSegment*> potentialSegments;
	potentialSegments.clear();

	for (CResponseSegment& currentSegment : * m_pChildSegments)
	{
		DRS_DEBUG_DATA_ACTION(AddResponseSegmentEvaluated(&currentSegment));
		u32k numConditions = currentSegment.GetNumConditions();
		if (numConditions < bestMatchedConditionsCountYet)
		{
			break;  //because the segments are ordered by their amount of conditions, we can stop looking for responses when we already found some with a higher condition count, it wont get any better
		}
		else if (currentSegment.AreConditionsMet(pResponseInstance))
		{
			potentialSegments.push_back(&currentSegment);  //we found a condition with more matched conditions than any other before
			bestMatchedConditionsCountYet = numConditions; //the new amount is our new reference, that every new candidate needs to have at least
		}
	}

	const size_t numPotentialSegments = potentialSegments.size();
	if (numPotentialSegments == 1) //trivial case, only one possible segment to execute found
	{
		return potentialSegments[0];
	}
	else if (numPotentialSegments > 1)  //if we have more than one condition with the same amount of matched conditions, we pick one at random
	{
		i32k randomPickedBestResponse = rand() % numPotentialSegments;
		return potentialSegments[randomPickedBestResponse];
	}
	return nullptr; //no valid segment found
}

//--------------------------------------------------------------------------------------------------
tukk CResponseSegment::GetName() const
{
#if !defined(_RELEASE)
	return m_name.c_str();
#else
	return "stripped";
#endif
}

//--------------------------------------------------------------------------------------------------
void CResponseSegment::UpdateName()
{
#if !defined(_RELEASE)
	//for display purpose we will simply add an automatically generated name to formally unnamed segments.
	if (m_name.empty() || strncmp(m_name.c_str(), "(AUTO) ", 6) == 0)
	{
		m_name = "(AUTO) ";
		i32k numActions = m_responseActions.size();
		if (numActions == 0)
		{
			m_name += "[no actions]";
		}
		else
		{
			m_name += "DO " + m_responseActions[0].m_pAction->GetVerboseInfoWithType();
			if (numActions == 2)
				m_name += " [+1 action]";
			else if (numActions > 1)
				m_name += " [+" + DrxStringUtils::toString(numActions - 1) + " actions]";
		}

		i32k numConditions = m_conditions.GetNumConditions();
		if (numConditions == 0)
		{
			m_name += ", [no conditions]";
		}
		else
		{
			if (m_conditions.IsNegated())
			{
				m_name += " NOT";
			}
			m_name += " IF ";
			if (m_conditions.IsConditionNegated(0))
			{
				m_name += "NOT ";
			}
			m_name += m_conditions[0]->GetVerboseInfoWithType();
			if (numConditions == 2)
				m_name += " [+1 condition]";
			else if (numConditions > 2)
				m_name += " [+" + DrxStringUtils::toString(numConditions - 1) + " conditions]";
		}
	}
#endif
}

//--------------------------------------------------------------------------------------------------
void CResponseSegment::ActionsInfo::Serialize(Serialization::IArchive& ar)
{
	if (ar.isInput())
		m_pAction = nullptr;

	ar(m_pAction, "Action", ">^");

	if (!m_pAction)
	{
#if !defined(_RELEASE)
		if (!CResponseUpr::s_currentSignal.empty())
		{
			m_pAction = DRS::IResponseActionSharedPtr(new CActionSpeakLine("li_" + CResponseUpr::s_currentSignal));
		}
		else
#endif
		{
			m_pAction = DRS::IResponseActionSharedPtr(new CActionSpeakLine("Fill me!"));
		}
		if (ar.isOutput())
		{
			ar(m_pAction, "Action", "^");
		}
	}

	ar(m_delay, "Delay", "^>Delay");
}

#if !defined(_RELEASE)
static i32 s_CurrentChildDepth = 0;
static i32 s_CurrentChildWidth[64] = { 0 };
static i32 s_DepthMaxWidth[64] = { 0 };
#endif

//--------------------------------------------------------------------------------------------------
void CResponseSegment::Serialize(Serialization::IArchive& ar)
{
#if !defined(_RELEASE)
	if (ar.isEdit())
	{
		string hierarchyName;
		if (s_CurrentChildDepth > 0)
		{
			s_CurrentChildWidth[s_CurrentChildDepth]++;
			for (i32 i = 1; i <= s_CurrentChildDepth; i++)
			{
				if (hierarchyName.empty())
					hierarchyName = "{Child";
				else
					hierarchyName += '.';
				hierarchyName += DrxStringUtils::toString(s_CurrentChildWidth[i]);
			}
			hierarchyName += "}";
		}
		else
		{
			hierarchyName = "{Base}";
		}
		ar(hierarchyName, "hierarchyName", "^!>80>");

		if (ar.isOutput())
		{
			UpdateName();
		}
	}
	if (!ar.caps(Serialization::IArchive::BINARY))
	{
		ar(m_name, "name", "^");
	}
#endif
	if (!ar.isEdit() || (ar.getFilter() & CResponseUpr::eSH_CollapsedResponseSegments) <= 0)
	{
		ar(m_conditions, "SegmentConditions", "^+<");

		if (ar.isEdit() && ar.isOutput())
		{
			std::sort(m_responseActions.begin(), m_responseActions.end());
		}
		ar(m_responseActions, "Actions", "Actions");
	}

	if (!m_pChildSegments)
	{
		m_pChildSegments = new ResponseSegmentsList();
	}
#if !defined(_RELEASE)
	s_CurrentChildDepth++;
	s_CurrentChildWidth[s_CurrentChildDepth] = 0;
	s_DepthMaxWidth[s_CurrentChildDepth] = m_pChildSegments->size();
#endif
	ar(*m_pChildSegments, "ChildSegments", "+<Follow up Responses");
#if !defined(_RELEASE)
	s_CurrentChildDepth--;
	if (ar.isEdit() && s_CurrentChildDepth > 0 && s_DepthMaxWidth[s_CurrentChildDepth] > s_CurrentChildWidth[s_CurrentChildDepth])
	{
		static string seperator = "______________________________________________________________________________________________________________________________________________________________________________________";
		ar(seperator, "seperator", "!<");
	}
#endif

	if (m_pChildSegments->empty())
	{
		delete m_pChildSegments;
		m_pChildSegments = nullptr;
	}
	else
	{
		std::sort(m_pChildSegments->begin(), m_pChildSegments->end()); //sort by amount of conditions, so that we can find the 'best' segment faster
	}
}

//--------------------------------------------------------------------------------------------------
bool CResponseSegment::operator<(const CResponseSegment& other) const
{
	return other.GetNumConditions() < GetNumConditions();
}
