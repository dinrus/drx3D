// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef STATOSCOPESTREAMINGINTERVALGROUP_H
#define STATOSCOPESTREAMINGINTERVALGROUP_H

#include <drx3D/Sys/Statoscope.h>

#include <drx3D/Sys/IStreamEngine.h>

#if ENABLE_STATOSCOPE

class CStatoscopeStreamingIntervalGroup : public CStatoscopeIntervalGroup, public IStreamEngineListener
{
public:
	enum
	{
		Stage_Waiting   = 0,
		Stage_IO        = (1 << 0),
		Stage_Inflate   = (1 << 1),
		Stage_Async     = (1 << 2),
		Stage_Preempted = (1 << 3),
		Stage_Decrypt   = (1 << 4),
	};

public:
	CStatoscopeStreamingIntervalGroup();

	void Enable_Impl();
	void Disable_Impl();

public: // IStreamEngineListener Members
	void OnStreamEnqueue(ukk pReq, tukk filename, EStreamTaskType source, const StreamReadParams& readParams);
	void OnStreamComputedSortKey(ukk pReq, uint64 key);
	void OnStreamBeginIO(ukk pReq, u32 compressedSize, u32 readSize, EStreamSourceMediaType mediaType);
	void OnStreamEndIO(ukk pReq);
	void OnStreamBeginInflate(ukk pReq);
	void OnStreamEndInflate(ukk pReq);
	void OnStreamBeginDecrypt(ukk pReq);
	void OnStreamEndDecrypt(ukk pReq);
	void OnStreamBeginAsyncCallback(ukk pReq);
	void OnStreamEndAsyncCallback(ukk pReq);
	void OnStreamDone(ukk pReq);
	void OnStreamPreempted(ukk pReq);
	void OnStreamResumed(ukk pReq);

private:
	void WriteChangeStageEvent(ukk pReq, i32 stage, bool entering);
};

#endif

#endif
