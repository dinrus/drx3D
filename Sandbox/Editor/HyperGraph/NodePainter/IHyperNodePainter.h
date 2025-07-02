// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HyperGraph/HyperGraphNode.h" // includes "DisplayList.h" and <GdiPlus.h>

struct IHyperNodePainter
{
	virtual void Paint(CHyperNode* pNode, CDisplayList* pList) = 0;
};


