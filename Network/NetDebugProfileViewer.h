// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETDEBUGPROFILEVIEWER_H__
#define __NETDEBUGPROFILEVIEWER_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_NET_DEBUG_INFO
	#if NET_PROFILE_ENABLE

		#include <drx3D/Network/NetDebugInfo.h>

class CNetDebugProfileViewer : public CNetDebugInfoSection
{
public:
	CNetDebugProfileViewer(u32 type);
	virtual ~CNetDebugProfileViewer();

	virtual void Tick(const SNetworkProfilingStats* const pProfilingStats);
	virtual void Draw(const SNetworkProfilingStats* const pProfilingStats);

private:

	void DrawProfileEntry(const SNetProfileStackEntry* entry, i32 depth, i32* line);
	void DrawTree(const SNetProfileStackEntry* root, i32 depth, i32* line);

};

	#endif
#endif // ENABLE_NET_DEBUG_INFO

#endif
