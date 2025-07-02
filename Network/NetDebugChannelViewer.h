// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETDEBUGCHANNELVIEWER_H__
#define __NETDEBUGCHANNELVIEWER_H__
#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_NET_DEBUG_INFO

	#include <drx3D/Network/NetDebugInfo.h>

///////////////////////////////////////////////////////////////////////////////

class CNetDebugChannelViewer : public CNetDebugInfoSection
{
public:
	CNetDebugChannelViewer();
	virtual ~CNetDebugChannelViewer();

	virtual void Tick(const SNetworkProfilingStats* const pNetworkProfilingStats) {};
	virtual void Draw(const SNetworkProfilingStats* const pDebugSpNetworkProfilingStatstats);
};

///////////////////////////////////////////////////////////////////////////////

class CNetDebugChannelViewerAccountingGroupsOverview : public CNetDebugInfoSection
{
public:
	CNetDebugChannelViewerAccountingGroupsOverview();
	virtual ~CNetDebugChannelViewerAccountingGroupsOverview();

	virtual void Tick(const SNetworkProfilingStats* const pNetworkProfilingStats) {};
	virtual void Draw(const SNetworkProfilingStats* const pNetworkProfilingStats);
};

///////////////////////////////////////////////////////////////////////////////

class CNetDebugChannelViewerAccountingGroupsSends : public CNetDebugInfoSection
{
public:
	CNetDebugChannelViewerAccountingGroupsSends();
	virtual ~CNetDebugChannelViewerAccountingGroupsSends();

	virtual void Tick(const SNetworkProfilingStats* const pNetworkProfilingStats) {};
	virtual void Draw(const SNetworkProfilingStats* const pNetworkProfilingStats);
};

///////////////////////////////////////////////////////////////////////////////

#endif // ENABLE_NET_DEBUG_INFO
#endif // __NETDEBUGCHANNELVIEWER_H__
