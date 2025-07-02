// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IHyperNodePainter.h"

class CHyperNodePainter_Default : public IHyperNodePainter
{
public:
	virtual void Paint(CHyperNode* pNode, CDisplayList* pList);
private:
	void         AddDownArrow(CDisplayList* pList, const Gdiplus::PointF& p, Gdiplus::Pen* pPen);
	void         CheckBreakpoint(IFlowGraphPtr pFlowGraph, const SFlowAddress& addr, bool& bIsBreakPoint, bool& bIsTracepoint);
};

static const float BREAKPOINT_X_OFFSET = 10.0f;
static const float MINIMIZE_BOX_MAX_HEIGHT = 12.0f;
static const float MINIMIZE_BOX_WIDTH = 16.0f;
static i32k PORTS_OUTER_MARGIN = 12;

