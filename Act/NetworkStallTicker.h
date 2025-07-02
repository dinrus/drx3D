// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETWORK_STALL_TICKER_H__
#define __NETWORK_STALL_TICKER_H__

#pragma once

//--------------------------------------------------------------------------
// a special ticker thread to run during load and unload of levels

#ifdef USE_NETWORK_STALL_TICKER_THREAD

	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/Sys/ICmdLine.h>

	#include <drx3D/CoreX/Game/IGameFramework.h>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CNetworkStallTickerThread : public IThread  //in multiplayer mode
{
public:
	CNetworkStallTickerThread()
	{
		m_threadRunning = true;
	}

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork()
	{
		m_threadRunning = false;
	}

private:
	 bool m_threadRunning;
};

#endif // #ifdef USE_NETWORK_STALL_TICKER_THREAD
//--------------------------------------------------------------------------

#endif
