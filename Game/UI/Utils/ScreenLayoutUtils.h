// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Screen Utilities such as safearea rendering.

-------------------------------------------------------------------------
История:
- 22:09:2009: Created by Frank Harrison for console TRC compliance.

*************************************************************************/

#ifndef __ScreenLayoutUtils_H__
#define __ScreenLayoutUtils_H__

class C2DRenderUtils;
class ScreenLayoutUpr;

#if ENABLE_HUD_EXTRA_DEBUG
class SafeAreaResizer;
#endif

class SafeAreaRenderer
{
public :
	SafeAreaRenderer( C2DRenderUtils* p2dRenderUtils, ScreenLayoutUpr* pScreenLayout );
	~SafeAreaRenderer( );

	void DrawSafeAreas( void );

private :

	void  DrawSafeArea( tukk text, const Vec2& border_pecentage );

	C2DRenderUtils* m_p2dRenderUtils;
	ScreenLayoutUpr* m_pScreenLayout;

#if ENABLE_HUD_EXTRA_DEBUG
	SafeAreaResizer* m_pResizer;

	i32 m_interactiveResize;
#endif
};

#endif // __ScreenLayoutUtils_H__