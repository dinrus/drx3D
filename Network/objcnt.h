// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __OBJCNT_H__
#define __OBJCNT_H__

#pragma once

#define ENABLE_OBJECT_COUNTING 0

#if !ENABLE_OBJECT_COUNTING
struct SCounter
{
	ILINE void operator++() {}
	ILINE void operator--() {}
	ILINE i32  QuickPeek()  { return 0; }
};
#else
struct SCounter
{
public:
	SCounter() : m_cnt(0) {}

	void operator++()
	{
		DrxInterlockedIncrement(&m_cnt);
	}

	void operator--()
	{
		DrxInterlockedDecrement(&m_cnt);
	}

	i32 QuickPeek()
	{
		return m_cnt;
	}

private:
	 i32 m_cnt;
};
#endif

struct SObjectCounters
{
#define COUNTER(name) SCounter name
#include <drx3D/Network/objcnt_defs.h>
#undef COUNTER
};

extern SObjectCounters g_objcnt;
extern SObjectCounters* g_pObjcnt;

#endif
