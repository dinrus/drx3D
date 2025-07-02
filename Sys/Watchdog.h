// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once
#include <drx3D/CoreX/Thread/IThreadUpr.h>

//! Runs in its own thread to watch over game freezes longer than user specified time out.
//! Turns time out into fatal error.
class CWatchdogThread : public IThread
{
public:

	void SignalStopWork();

	//! Creates and starts running watchdog thread.
	//! \param timeOutSeconds	time out value in seconds, must be positive
	explicit CWatchdogThread(i32 timeOutSeconds);
	~CWatchdogThread();

	//! Changes time out value.
	//! \param timeOutSeconds	time out value in seconds, must be positive
	void SetTimeout(i32 timeOutSeconds);
private:

	static uint64 GetSystemUpdateCounter();
	void          Sleep() const;

	virtual void  ThreadEntry() override;

	 bool m_bQuit = false;
	i32           m_timeOutSeconds = 0;
};
