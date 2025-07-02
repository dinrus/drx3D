// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(ENABLE_LOADING_PROFILER)

#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CBootProfilerRecord;
class CBootProfilerSession;

enum class EBootProfilerFormat : i32
{
	XML,
	ChromeTraceJSON
};

//////////////////////////////////////////////////////////////////////////

class CBootProfiler : public ISystemEventListener, public IThread
{
	friend class CBootProfileBLock;
	friend class CBootProfilerSession;
public:

	struct SSaveSessionInfo
	{
		float                 functionMinTimeThreshold;
		CBootProfilerSession* pSession;

		SSaveSessionInfo(float functionMinTimeThreshold, CBootProfilerSession* pSession)
			: functionMinTimeThreshold(functionMinTimeThreshold)
			, pSession(pSession)
		{}
	};

	typedef DrxMT::vector<CBootProfilerSession*> TSessions;
	typedef DrxMT::vector<SSaveSessionInfo> TSessionsToSave;

public:
	CBootProfiler();
	~CBootProfiler();

	// IThread
	virtual void               ThreadEntry() override;
	// ~IThread

	static CBootProfiler&      GetInstance();

	void                       Init(ISystem* pSystem);
	void                       RegisterCVars();

	void                       StartSession(tukk sessionName);
	void                       StopSession();

	void                       StopSaveSessionsThread();
	void                       QueueSessionToDelete(CBootProfilerSession*pSession);
	void                       QueueSessionToSave(float functionMinTimeThreshold, CBootProfilerSession* pSession);

	CBootProfilerRecord*       StartBlock(tukk name, tukk args,EProfileDescription type);
	void                       StopBlock(CBootProfilerRecord* record);

	void                       StartFrame(tukk name);
	void                       StopFrame();

protected:
	// ISystemEventListener
	virtual void               OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

private:
	CBootProfilerSession*      m_pCurrentSession;

	bool                       m_quitSaveThread;
	DrxEvent                   m_saveThreadWakeUpEvent;
	TSessions                  m_sessionsToDelete;
	TSessionsToSave            m_sessionsToSave;

	static EBootProfilerFormat CV_sys_bp_output_formats;
	static i32                 CV_sys_bp_enabled;
	static i32                 CV_sys_bp_level_load;
	static i32                 CV_sys_bp_frames_worker_thread;
	static i32                 CV_sys_bp_frames;
	static i32                 CV_sys_bp_frames_sample_period;
	static i32                 CV_sys_bp_frames_sample_period_rnd;
	static float               CV_sys_bp_frames_threshold;
	static float               CV_sys_bp_time_threshold;
	CBootProfilerRecord*       m_pMainThreadFrameRecord;

	i32                        m_levelLoadAdditionalFrames;
	i32                        m_countdownToNextSaveSesssion;
};

#else //ENABLE_LOADING_PROFILER

#endif ////ENABLE_LOADING_PROFILER
