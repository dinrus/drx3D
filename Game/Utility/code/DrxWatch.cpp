// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
DrxWatch.cpp

Описание: 
	- basic onscreen watch
	- in game.dll till it matures and can be moved into the engine

-------------------------------------------------------------------------
История:
-	[03/08/2009] : Created by James Bamford

*************************************************************************/


#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/DrxWatch.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/CoreX/String/StringUtils.h>

#if DRX_WATCH_ENABLED

static i32 s_watchTextLastPrintedDuringFrame = -1;
static float s_watchTextYPos = 0.f;
static float s_watchTextXPos = 0.f;
static float s_max_width_this_col = 0.f;

static float GetWatchTextYPos()
{
	i32 frame = gEnv->pRenderer->GetFrameID(false);

	// Apply overscan borders to console text pos when console is visible
	float belowConsolePosY = g_pGameCVars->watch_text_render_start_pos_y;
	const bool bConsoleVisible = GetISystem()->GetIConsole()->GetStatus();
	if(bConsoleVisible)
	{
		const float screenHeight = (float)gEnv->pRenderer->GetHeight();
		Vec2 overscanBorders = Vec2(0.0f,0.0f);
		gEnv->pRenderer->EF_Query(EFQ_OverscanBorders, overscanBorders);
		const float overscanBorderHeight = screenHeight * overscanBorders.y;
		const float consoleOffset = 5.0f;
		belowConsolePosY = (screenHeight * 0.5f) + consoleOffset - overscanBorderHeight;
	}

	if (s_watchTextLastPrintedDuringFrame == frame)
	{
		s_watchTextYPos += g_pGameCVars->watch_text_render_size * g_pGameCVars->watch_text_render_lineSpacing;
		if( s_watchTextYPos + (g_pGameCVars->watch_text_render_size * g_pGameCVars->watch_text_render_lineSpacing) > gEnv->pRenderer->GetHeight() )
		{
			s_watchTextYPos = belowConsolePosY;
			s_watchTextXPos += s_max_width_this_col + 15;
			s_max_width_this_col = 0.f;
		}
	}
	else
	{
		s_watchTextLastPrintedDuringFrame = frame;
		s_watchTextYPos = belowConsolePosY;
		s_watchTextXPos = 0.f;
	}

	return s_watchTextYPos;
}

i32 DrxWatchFunc (tukk  message)
{
	// Fran: we need these guards for the testing framework to work
	if (gEnv && gEnv->pRenderer && g_pGameCVars && g_pGameCVars->watch_enabled)
	{
		float color[4] = {1,1,1,1};
		IFFont* pFont = gEnv->pDrxFont->GetFont("default");
		float xscale = g_pGameCVars->watch_text_render_size*g_pGameCVars->watch_text_render_fxscale;
		STextDrawContext ctx;
		ctx.SetSize(Vec2(xscale, xscale));
		float width = pFont->GetTextSize(message, true, ctx).x;
		if( width>s_max_width_this_col )
			s_max_width_this_col = width;
		float yPos = GetWatchTextYPos(); // also updates s_watchTextXPos
		gEnv->pRenderer->Draw2dLabel(g_pGameCVars->watch_text_render_start_pos_x+s_watchTextXPos, yPos, g_pGameCVars->watch_text_render_size, color, false, "%s", message);
		return 1;
	}

	return 0;
}

i32 DrxWatchLogFunc (tukk  message)
{
	DrxWatchFunc(message);
	DrxLog("%s", message);

	return 0;
}

//======================================================================================
// Lingering 3D watches...
//======================================================================================

struct SLingeringWatch3D 
{
	char m_text[16];
	float m_timeLeft;
	float m_gravity;
	Vec3 m_pos;
	Vec3 m_vel;
};

#define MAX_LINGERING_WATCH_3D	8
static SLingeringWatch3D s_lingeringWatch3D[MAX_LINGERING_WATCH_3D];
static i32 s_lingeringWatch3D_num = 0;

void DrxWatch3DAdd(tukk  text, const Vec3 & posIn, float lifetime, const Vec3 * velocity, float gravity)
{
	if (text && text[0])
	{
		SLingeringWatch3D * slot = & s_lingeringWatch3D[s_lingeringWatch3D_num];
		s_lingeringWatch3D_num = (s_lingeringWatch3D_num + 1) % MAX_LINGERING_WATCH_3D;
		drx_strcpy(slot->m_text, text);
		slot->m_timeLeft = lifetime;
		slot->m_gravity = gravity;
		slot->m_pos = posIn;
		slot->m_vel = velocity ? *velocity : Vec3(0.f, 0.f, 0.f);
	}
};

void DrxWatch3DReset()
{
	memset (& s_lingeringWatch3D, 0, sizeof(s_lingeringWatch3D));
}

void DrxWatch3DTick(float dt)
{
	for (i32 i = 0; i < MAX_LINGERING_WATCH_3D; ++ i)
	{
		if (s_lingeringWatch3D[i].m_text[0])
		{
			float fadeaway = min(1.f, s_lingeringWatch3D[i].m_timeLeft);
			const float col[] = {1.f, 1.f, 1.f, fadeaway};
			gEnv->pRenderer->DrawLabelEx(s_lingeringWatch3D[i].m_pos, 3.f, col, false, true, "%s", s_lingeringWatch3D[i].m_text);

			if (s_lingeringWatch3D[i].m_timeLeft > dt)
			{
				s_lingeringWatch3D[i].m_timeLeft -= dt;
				s_lingeringWatch3D[i].m_pos += s_lingeringWatch3D[i].m_vel * dt;
				s_lingeringWatch3D[i].m_vel.z -= dt * s_lingeringWatch3D[i].m_gravity;
			}
			else
			{
				s_lingeringWatch3D[i].m_timeLeft = 0;
				s_lingeringWatch3D[i].m_text[0] = '\0';
			}
		}
	}
}

#endif // DRX_WATCH_ENABLED
