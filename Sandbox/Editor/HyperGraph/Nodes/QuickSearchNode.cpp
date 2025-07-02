// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "QuickSearchNode.h"

#include "HyperGraph/NodePainter/HyperNodePainter_QuickSearch.h"


static CHyperNodePainter_QuickSearch painter;

CQuickSearchNode::CQuickSearchNode()
	: m_iSearchResultCount(1)
	, m_iIndex(1)
{
	SetClass(GetClassType());
	m_pPainter = &painter;
	m_name = "Game:Start";
}

CHyperNode* CQuickSearchNode::Clone()
{
	CQuickSearchNode* pNode = new CQuickSearchNode();
	pNode->CopyFrom(*this);
	return pNode;
}

