// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IHyperNodePainter.h"

class CHyperNodePainter_Comment : public IHyperNodePainter
{
public:
	virtual void Paint(CHyperNode* pNode, CDisplayList* pList);
};


