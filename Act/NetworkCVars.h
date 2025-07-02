// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETWORKCVARS_H__
#define __NETWORKCVARS_H__

#pragma once

#include <drx3D/Sys/IConsole.h>

class CNetworkCVars
{
public:
	i32   BreakageLog;
	float VoiceVolume;
	float PhysSyncPingSmooth;
	float PhysSyncLagSmooth;
	i32   PhysDebug;
	i32   BreakTimeoutFrames;
	float BreakMaxWorldSize;
	float BreakWorldOffsetX;
	float BreakWorldOffsetY;
	i32   sv_LoadAllLayersForResList;

	static ILINE CNetworkCVars& Get()
	{
		DRX_ASSERT(s_pThis);
		return *s_pThis;
	}

private:
	friend class CDrxAction; // Our only creator

	CNetworkCVars(); // singleton stuff
	~CNetworkCVars();
	CNetworkCVars(const CNetworkCVars&);
	CNetworkCVars& operator=(const CNetworkCVars&);

	static CNetworkCVars* s_pThis;
};

#endif
