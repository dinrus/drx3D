// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HyperGraph/HyperGraphNode.h"

class CQuickSearchNode : public CHyperNode
{
public:
	static tukk GetClassType()
	{
		return "QuickSearch";
	}
	CQuickSearchNode();

	// CHyperNode
	void         Init() override                      {}
	CHyperNode*  Clone() override;

	virtual bool IsEditorSpecialNode() const override { return true; }
	virtual bool IsFlowNode() const override          { return false; }
	// ~CHyperNode

	void         SetSearchResultCount(i32 count)      { m_iSearchResultCount = count; };
	i32          GetSearchResultCount()               { return m_iSearchResultCount; };

	void         SetIndex(i32 index)                  { m_iIndex = index; };
	i32          GetIndex()                           { return m_iIndex; };

private:
	i32 m_iSearchResultCount;
	i32 m_iIndex;
};

