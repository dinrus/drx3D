// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DrxThread_posix.h"

//////////////////////////////////////////////////////////////////////////
// DrxEvent(Timed) implementation
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void DrxEventTimed::Reset()
{
	m_lockNotify.Lock();
	m_flag = false;
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void DrxEventTimed::Set()
{
	m_lockNotify.Lock();
	m_flag = true;
	m_cond.Notify();
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void DrxEventTimed::Wait()
{
	m_lockNotify.Lock();
	if (!m_flag)
		m_cond.Wait(m_lockNotify);
	m_flag = false;
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
bool DrxEventTimed::Wait(u32k timeoutMillis)
{
	bool bResult = true;
	m_lockNotify.Lock();
	if (!m_flag)
		bResult = m_cond.TimedWait(m_lockNotify, timeoutMillis);
	m_flag = false;
	m_lockNotify.Unlock();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// DrxCriticalSection implementation
///////////////////////////////////////////////////////////////////////////////
typedef DrxLockT<DRXLOCK_RECURSIVE> TCritSecType;

void DrxDeleteCriticalSection(uk cs)
{
	delete ((TCritSecType*)cs);
}

void DrxEnterCriticalSection(uk cs)
{
	((TCritSecType*)cs)->Lock();
}

bool DrxTryCriticalSection(uk cs)
{
	return false;
}

void DrxLeaveCriticalSection(uk cs)
{
	((TCritSecType*)cs)->Unlock();
}

void DrxCreateCriticalSectionInplace(uk pCS)
{
	new(pCS) TCritSecType;
}

void DrxDeleteCriticalSectionInplace(uk )
{
}

uk DrxCreateCriticalSection()
{
	return (uk ) new TCritSecType;
}
