// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Раелизация консоли для Android, ведёт отчёт на главный интерфейс.
   -------------------------------------------------------------------------
   История:
   - Aug 26,2013:	Created by Leander Beernaert

*************************************************************************/
#if defined(ANDROID)

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/AndroidConsole.h>

#include "android/log.h"

CAndroidConsole::CAndroidConsole() :
	m_isInitialized(false)
{

}

CAndroidConsole::~CAndroidConsole()
{

}

// Interface IOutputPrintSink /////////////////////////////////////////////
void CAndroidConsole::Print(tukk line)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "DinrusX", "MSG: %s\n", line);
}
// Interface ISystemUserCallback //////////////////////////////////////////
bool CAndroidConsole::OnError(tukk errorString)
{
	__android_log_print(ANDROID_LOG_ERROR, "DinrusX", "ERR: %s\n", errorString);
	return true;
}

void CAndroidConsole::OnInitProgress(tukk sProgressMsg)
{
	(void) sProgressMsg;
	// Do Nothing
}
void CAndroidConsole::OnInit(ISystem* pSystem)
{
	if (!m_isInitialized)
	{
		IConsole* pConsole = pSystem->GetIConsole();
		if (pConsole != 0)
		{
			pConsole->AddOutputPrintSink(this);
		}
		m_isInitialized = true;
	}
}
void CAndroidConsole::OnShutdown()
{
	if (m_isInitialized)
	{
		// remove outputprintsink
		m_isInitialized = false;
	}
}
void CAndroidConsole::OnUpdate()
{
	// Do Nothing
}
void CAndroidConsole::GetMemoryUsage(IDrxSizer* pSizer)
{
	size_t size = sizeof(*this);

	pSizer->AddObject(this, size);
}

// Interface ITextModeConsole /////////////////////////////////////////////
Vec2_tpl<i32> CAndroidConsole::BeginDraw()
{
	return Vec2_tpl<i32>(0, 0);
}
void CAndroidConsole::PutText(i32 x, i32 y, tukk msg)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "DinrusX", "PUT: %s\n", msg);
}
void CAndroidConsole::EndDraw()
{
	// Do Nothing
}
#endif // ANDROID
