// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INETCONTEXTSTATE_H__
#define __INETCONTEXTSTATE_H__

#pragma once

struct INetContextState : public CMultiThreadRefCount
{
	virtual ~INetContextState() {}
	virtual void Die() = 0;
};

#endif // __INETCONTEXTSTATE_H__
