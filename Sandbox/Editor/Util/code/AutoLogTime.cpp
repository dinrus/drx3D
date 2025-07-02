// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <stdafx.h>
#include "AutoLogTime.h"

CAutoLogTime::CAutoLogTime(tukk what)
{
	m_what = what;
	DrxLog("---- Start: %s", m_what);
	m_t0 = GetTickCount();
}

CAutoLogTime::~CAutoLogTime()
{
	m_t1 = GetTickCount();
	DrxLog("---- End: %s (%d seconds)", m_what, (m_t1 - m_t0) / 1000);
}

