// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IHyperNodePainter.h"

class CHyperNodePainter_CommentBox : public IHyperNodePainter
{
public:
	CHyperNodePainter_CommentBox()
		: m_brushSolid(Gdiplus::Color::Black)
		, m_brushTransparent(Gdiplus::Color::Black)
	{}
	virtual void  Paint(CHyperNode* pNode, CDisplayList* pList);
	static float& AccessStaticVar_ZoomFactor() { static float m_zoomFactor(1.f); return m_zoomFactor; }

private:
	Gdiplus::SolidBrush m_brushSolid;
	Gdiplus::SolidBrush m_brushTransparent;
};

