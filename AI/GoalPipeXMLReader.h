// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GoalPipeXMLReader_H__
#define __GoalPipeXMLReader_H__

#include <drx3D/AI/IGoalPipe.h>

class CPipeUpr;

class CGoalPipeXMLReader
{
public:
	CGoalPipeXMLReader();

	bool LoadGoalPipesFromXmlFile(tukk filename);
	bool LoadGoalPipesFromXmlNode(const XmlNodeRef& root);

	void ParseGoalPipe(tukk szGoalPipeName, const XmlNodeRef goalPipeNode, CPipeUpr::ActionToTakeWhenDuplicateFound actionToTakeWhenDuplicateFound = CPipeUpr::ReplaceDuplicateAndReportError);

private:
	void   ParseGoalOps(IGoalPipe* pGoalPipe, const XmlNodeRef& root,
	                    string sIfLabel = string(), bool b_IfElseEnd_Halves_ReverseOrder = true);
	void   ParseGoalOp(IGoalPipe* pGoalPipe, const XmlNodeRef& goalOpNode);

	string GenerateUniqueLabelName();

private:
	class CXMLAttrReader
	{
		typedef std::pair<tukk , i32> TRecord;
		std::vector<TRecord> m_dictionary;

	public:
		void Add(tukk szKey, i32 nValue) { m_dictionary.push_back(TRecord(szKey, nValue)); }
		bool Get(tukk szKey, i32& nValue);
	};

	CXMLAttrReader        m_dictBranchType;

	IGoalPipe::EGroupType m_currentGrouping;

	u32                m_nextUniqueLabelID;
};

#endif  // #ifndef __GoalPipeXMLReader_H__
