// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   The Response Segment is responsible for holding and checking the conditions for this segment
   and to hold and execute the actions

************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include "ConditionsCollection.h"

namespace DrxDRS
{
class CResponseInstance;
class CConditionsCollection;
class CVariableCollection;

class CResponseSegment
{
public:
	friend class CDataImportHelper;

	struct ActionsInfo
	{
		ActionsInfo() : m_delay(0.0f) {}
		DRS::IResponseActionSharedPtr m_pAction;
		float                         m_delay;

		void Serialize(Serialization::IArchive& ar);
		bool operator<(const ActionsInfo& other) const { return m_delay < other.m_delay; }
	};
	typedef std::vector<ActionsInfo> ActionsList;

	CResponseSegment();
	CResponseSegment(CResponseSegment&& other);
	~CResponseSegment();
	CResponseSegment& operator=(CResponseSegment&& other);

	void              AddAction(DRS::IResponseActionSharedPtr pAction);
	void              AddCondition(DRS::IConditionSharedPtr pCondition, bool Negated);
	u32            GetNumActions() const                                  { return (u32)m_responseActions.size(); }
	u32            GetNumConditions() const                               { return m_conditions.GetNumConditions(); }
	ActionsList&      GetActions()                                           { return m_responseActions; }
	bool              AreConditionsMet(CResponseInstance* pResponseInstance) { return m_conditions.IsMet(pResponseInstance); }

	CResponseSegment* GetNextResponseSegment(CResponseInstance* pResponseInstance);
	tukk       GetName() const;

	void              Serialize(Serialization::IArchive& ar);
	bool              operator<(const CResponseSegment& other) const; //Remark: we sort (child)segments by their amount of conditions, this makes it easier for us to find the 'best' one

private:
	void UpdateName();

	typedef std::vector<CResponseSegment> ResponseSegmentsList;

	ActionsList           m_responseActions;
	CConditionsCollection m_conditions;
	ResponseSegmentsList* m_pChildSegments;
#if !defined(_RELEASE)
	string                m_name;
#endif
};
} //namespace DrxDRS
