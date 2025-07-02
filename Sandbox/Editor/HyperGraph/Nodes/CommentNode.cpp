// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "CommentNode.h"

#include "HyperGraph/NodePainter/HyperNodePainter_Comment.h"

static CHyperNodePainter_Comment comment_painter;

CCommentNode::CCommentNode()
{
	SetClass(GetClassType());
	m_pPainter = &comment_painter;
	m_name = "This is a comment";
}

CHyperNode* CCommentNode::Clone()
{
	CCommentNode* pNode = new CCommentNode();
	pNode->CopyFrom(*this);
	return pNode;
}

