// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef AutoLogTime_h__
#define AutoLogTime_h__

#pragma once

class CAutoLogTime
{
public:
	CAutoLogTime(tukk what);
	~CAutoLogTime();
private:
	tukk m_what;
	i32         m_t0, m_t1;
};

#endif // AutoLogTime_h__

