// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrawContext.h
//  Created:     26/08/2009 by Timur.
//  Описание: DrawContext helper class for MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MiniGUI_DrawContext_h__
#define __MiniGUI_DrawContext_h__

#include <drx3D/Sys/IDrxMiniGUI.h>
#include <drx3D/CoreX/Math/Drx_Color.h>

struct IRenderAuxGeom;

MINIGUI_BEGIN

enum ETextAlign
{
	eTextAlign_Left,
	eTextAlign_Right,
	eTextAlign_Center
};

//////////////////////////////////////////////////////////////////////////
// Context of MiniGUI drawing.
//////////////////////////////////////////////////////////////////////////
class CDrawContext
{
public:
	CDrawContext(SMetrics* pMetrics);

	// Must be called before any drawing happens
	void      StartDrawing();
	// Must be called after all drawing have been complete.
	void      StopDrawing();

	void      PushClientRect(const Rect& rc);
	void      PopClientRect();

	SMetrics& Metrics() { return *m_pMetrics; }
	void      SetColor(ColorB color);

	void      DrawLine(float x0, float y0, float x1, float y1, float thickness = 1.0f);
	void      DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2);
	void      DrawRect(const Rect& rc);
	void      DrawFrame(const Rect& rc, ColorB lineColor, ColorB solidColor, float thickness = 1.0f);

	void      DrawString(float x, float y, float font_size, ETextAlign align, tukk format, ...);

protected:
	SMetrics*       m_pMetrics;

	ColorB          m_color;
	float           m_defaultZ;
	IRenderAuxGeom* m_pAuxRender;
	u32          m_prevRenderFlags;

	enum { MAX_ORIGIN_STACK = 16 };

	i32   m_currentStackLevel;
	float m_x, m_y; // Reference X,Y positions
	Rect  m_clientRectStack[MAX_ORIGIN_STACK];

	float m_frameWidth;
	float m_frameHeight;
};

MINIGUI_END

#endif //__MiniGUI_DrawContext_h__
