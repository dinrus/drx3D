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
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ScreenLayoutUtils.h>
#include <drx3D/Game/Graphics/2DRenderUtils.h>
#include <drx3D/Game/ScreenLayoutUpr.h>
#include <drx3D/Input/IInput.h>
#include <drx3D/Input/IHardwareMouse.h>
#include <drx3D/Game/GameActions.h>

#if ENABLE_HUD_EXTRA_DEBUG
class SafeAreaResizer : public IHardwareMouseEventListener
{
public :
	SafeAreaResizer( ScreenLayoutUpr* pLayoutMan )
	: m_layoutMan(pLayoutMan)
	{
		SAFE_HARDWARE_MOUSE_FUNC(AddListener(this));
		g_pGameActions->FilterNoMouse()->Enable(true);
	}
	
	~SafeAreaResizer()
	{
		SAFE_HARDWARE_MOUSE_FUNC(RemoveListener(this));
		g_pGameActions->FilterNoMouse()->Enable(false);
	}

// IHardwareMouseEventListener
	void OnHardwareMouseEvent(i32 iX,i32 iY,EHARDWAREMOUSEEVENT eHardwareMouseEvent, i32 wheelDelta = 0)
	{
		Vec2 screenFraction(0,0);
		screenFraction.x = (float)iX / (float)gEnv->pRenderer->GetWidth();
		screenFraction.y = (float)iY / (float)gEnv->pRenderer->GetHeight();

		m_layoutMan->SetSafeArea( screenFraction );
	}
//~IHardwareMouseEventListener
	
	ScreenLayoutUpr* m_layoutMan;
};

#endif // ENABLE_HUD_EXTRA_DEBUG

//---------------------------------------------------
SafeAreaRenderer::SafeAreaRenderer( C2DRenderUtils* p2dRenderUtils, ScreenLayoutUpr* pScreenLayout )
{
	m_p2dRenderUtils = p2dRenderUtils;
	m_pScreenLayout = pScreenLayout;

#if ENABLE_HUD_EXTRA_DEBUG
	m_interactiveResize = 0;
	m_pResizer=NULL;

	REGISTER_CVAR2("HUD_interactiveSafeAreas", &m_interactiveResize, 0, 0, "Interactively resize safe areas.");
#endif
}

//---------------------------------------------------
SafeAreaRenderer::~SafeAreaRenderer( )
{
	//...
}

//---------------------------------------------------
void SafeAreaRenderer::DrawSafeAreas( void )
{
#if ENABLE_HUD_EXTRA_DEBUG
	if( m_interactiveResize && !m_pResizer )
	{
		m_pResizer = new SafeAreaResizer(m_pScreenLayout);
	}
	else if(m_pResizer && !m_interactiveResize)
	{
		SAFE_DELETE( m_pResizer );
	}
#endif

	ScreenLayoutUpr* pLayout = m_pScreenLayout;
	const Vec2 fullscreen_border = pLayout->GetSafeAreaBorderScreenProportion( eHSAID_fullscreen );
	const Vec2 title_safe_border = pLayout->GetSafeAreaBorderScreenProportion( eHSAID_title );
	const Vec2 sony_trc_safe_border = pLayout->GetSafeAreaBorderScreenProportion( eHSAID_PS4_trc );
	const Vec2 action_safe_border = pLayout->GetSafeAreaBorderScreenProportion( eHSAID_action );

	// Get state so we can reset later
	ScreenLayoutStates prev_state = pLayout->GetState();

	// Set to null state so we can render full screen.
	pLayout->SetState(eSLO_DoNotAdaptToSafeArea|eSLO_ScaleMethod_None);

	DrawSafeArea( string().Format( "fullscreen area : should not see me! %d%%x,%d%%y", (i32)(100.0f*fullscreen_border.x), (i32)(100.0f*fullscreen_border.y) ).c_str(), fullscreen_border    );
	DrawSafeArea( string().Format( "NTSC/PAL title safe %d%%x,%d%%y",  (i32)(100.0f*title_safe_border.x),    (i32)(100.0f*title_safe_border.y) ).c_str(),              title_safe_border    );
	DrawSafeArea( string().Format( "sony trc safe %d%%x,%d%%y",        (i32)(100.0f*sony_trc_safe_border.x), (i32)(100.0f*sony_trc_safe_border.y) ).c_str(),           sony_trc_safe_border );
	DrawSafeArea( string().Format( "NTSC/PAL action safe %d%%x,%d%%y", (i32)(100.0f*action_safe_border.x),   (i32)(100.0f*action_safe_border.y) ).c_str(),             action_safe_border   );
	if( pLayout->GetCurSafeAreaId() == eHSAID_custom )
	{
		const Vec2 custom_safe_border = pLayout->GetSafeAreaBorderScreenProportion( eHSAID_custom );
		DrawSafeArea( string().Format( "Custom safe %d%%x,%d%%y (%d,%d)pixels", (i32)(100.0f*custom_safe_border.x), (i32)(100.0f*custom_safe_border.y), (i32)(gEnv->pRenderer->GetWidth()*(1.0f-(2.0f*custom_safe_border.x))), (i32)(gEnv->pRenderer->GetHeight()*(1.0f-(2.0f*custom_safe_border.y)) ) ).c_str(), custom_safe_border );// , (i32)(100.0f*m_customSafeArea))
	}

	// Size debug
	const float screen_width  = (float)pLayout->GetVirtualWidth();
	const float screen_height = (float)pLayout->GetVirtualHeight();
	const Vec2 cur_safe = pLayout->GetSafeAreaBorderScreenProportion( pLayout->GetCurSafeAreaId() );
	const Vec2 orig( cur_safe.x * screen_width, cur_safe.y * screen_height );
	const Vec2 max( screen_width - (cur_safe.x * screen_width), screen_height - (cur_safe.y * screen_height) );

	ColorF pos_text(1.0f, 1.0f, 1.0f, 1.0f);
	m_p2dRenderUtils->SetFont( gEnv->pDrxFont->GetFont("default") );
	m_p2dRenderUtils->DrawText( orig.x, orig.y, 15.0f, 15.0f, "(0,0)", pos_text);
	m_p2dRenderUtils->DrawText(  max.x,  max.y, 15.0f, 15.0f, "(800,600)", pos_text, UIDRAWHORIZONTAL_RIGHT, UIDRAWVERTICAL_BOTTOM);

	pLayout->SetState(prev_state);
}

//---------------------------------------------------
void SafeAreaRenderer::DrawSafeArea( tukk text, const Vec2& border_pecentage)
{    
	ScreenLayoutUpr* pLayout = m_pScreenLayout;

	const Vec2 cur_safe = pLayout->GetSafeAreaBorderScreenProportion( pLayout->GetCurSafeAreaId() );

	//                                    good   bad
	const bool inSafeArea = (border_pecentage.x>=cur_safe.x);
	float r = inSafeArea ? 0.0f : 1.0f;
	float g = inSafeArea ? 1.0f : 0.0f;
	float b = 0.06f;
	float a = inSafeArea ? 0.5f : 1.0f;
	ColorF screen_edge_color( r, g, b, a );

	const float screen_width = (float)pLayout->GetVirtualWidth();
	const float screen_height = (float)pLayout->GetVirtualHeight();

	const float x_safe  = border_pecentage.x * screen_width;
	const float y_safe = border_pecentage.y * screen_height;
	const float x1 = x_safe;
	const float x2 = screen_width - x_safe;
	const float y1 = y_safe;
	const float y2 = screen_height -y_safe;

	m_p2dRenderUtils->DrawRect( x1, y1, x2-x1, y2-y1, screen_edge_color );
	// Label
	m_p2dRenderUtils->SetFont(gEnv->pDrxFont->GetFont("default"));
	m_p2dRenderUtils->DrawText( x_safe, screen_height - y_safe, 
	                            15.0f, 15.0f, 
	                            text, screen_edge_color );

}