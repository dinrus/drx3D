// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HyperGraph/HyperGraphNode.h"

class CCommentNode : public CHyperNode
{
public:
	static tukk GetClassType()
	{
		return "_comment";
	}
	CCommentNode();

	// CHyperNode
	void                Init() override                      {}
	CHyperNode*         Clone() override;

	virtual bool        IsEditorSpecialNode() const override { return true; }
	virtual bool        IsFlowNode() const override          { return false; }
	virtual tukk GetDescription() const override      { return "Simple Comment."; }
	virtual tukk GetInfoAsString() const override     { return "Class: Comment"; }
	// ~CHyperNode
};

