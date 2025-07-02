// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __HANDLERBASE_H__
#define __HANDLERBASE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

i32k MAX_CLIENTS_NUM = 100;

struct HandlerBase
{
	HandlerBase(tukk bucket, i32 affinity);
	~HandlerBase();

	void   SetAffinity();
	u32 SyncSetAffinity(u32 cpuMask);

	string m_serverLockName;
	string m_clientLockName;
	u32 m_affinity;
	u32 m_prevAffinity;
};

#endif // __HANDLERBASE_H__
