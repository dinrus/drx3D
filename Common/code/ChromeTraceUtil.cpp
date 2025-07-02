
#include <drx3D/Common/ChromeTraceUtil.h>
#include <drx3D/Common/b3Clock.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Common/b3Logging.h>
#include <stdio.h>
#include <climits>

struct Timing
{
	tukk m_name;
	i32 m_threadId;
	zu64 m_usStartTime;
	zu64 m_usEndTime;
};

FILE* gTimingFile = 0;
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif  //__STDC_FORMAT_MACROS

//see http://stackoverflow.com/questions/18107426/printf-format-for-unsigned-int64-on-windows
#ifndef _WIN32
#include <inttypes.h>
#endif

#define DRX3D_TIMING_CAPACITY 16 * 65536
static bool m_firstTiming = true;

struct Timings
{
	Timings()
		: m_numTimings(0),
		  m_activeBuffer(0)
	{
	}
	void flush()
	{
		for (i32 i = 0; i < m_numTimings; i++)
		{
			tukk name = m_timings[m_activeBuffer][i].m_name;
			i32 threadId = m_timings[m_activeBuffer][i].m_threadId;
			zu64 startTime = m_timings[m_activeBuffer][i].m_usStartTime;
			zu64 endTime = m_timings[m_activeBuffer][i].m_usEndTime;

			if (!m_firstTiming)
			{
				fprintf(gTimingFile, ",\n");
			}

			m_firstTiming = false;

			if (startTime > endTime)
			{
				endTime = startTime;
			}

			zu64 startTimeDiv1000 = startTime / 1000;
			zu64 endTimeDiv1000 = endTime / 1000;

#if 0

			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%" PRIu64 ".123 ,\"ph\":\"B\",\"name\":\"%s\",\"args\":{}},\n",
				threadId, startTimeDiv1000, name);
			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%" PRIu64 ".234 ,\"ph\":\"E\",\"name\":\"%s\",\"args\":{}}",
				threadId, endTimeDiv1000, name);

#else

			
			u32 startTimeRem1000 = startTime % 1000;
			u32 endTimeRem1000 = endTime % 1000;

			char startTimeRem1000Str[16];
			char endTimeRem1000Str[16];

			if (startTimeRem1000 < 10)
			{
				sprintf(startTimeRem1000Str, "00%d", startTimeRem1000);
			}
			else
			{
				if (startTimeRem1000 < 100)
				{
					sprintf(startTimeRem1000Str, "0%d", startTimeRem1000);
				}
				else
				{
					sprintf(startTimeRem1000Str, "%d", startTimeRem1000);
				}
			}

			if (endTimeRem1000 < 10)
			{
				sprintf(endTimeRem1000Str, "00%d", endTimeRem1000);
			}
			else
			{
				if (endTimeRem1000 < 100)
				{
					sprintf(endTimeRem1000Str, "0%d", endTimeRem1000);
				}
				else
				{
					sprintf(endTimeRem1000Str, "%d", endTimeRem1000);
				}
			}

			char newname[1024];
			static i32 counter2 = 0;
			sprintf(newname, "%s%d", name, counter2++);

#ifdef _WIN32
			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%I64d.%s ,\"ph\":\"B\",\"name\":\"%s\",\"args\":{}},\n",
					threadId, startTimeDiv1000, startTimeRem1000Str, newname);
			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%I64d.%s ,\"ph\":\"E\",\"name\":\"%s\",\"args\":{}}",
					threadId, endTimeDiv1000, endTimeRem1000Str, newname);
#else
			// Note: on 64b build, PRIu64 resolves in 'lu' whereas timings ('ts') have to be printed as 'llu'.
			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%llu.%s ,\"ph\":\"B\",\"name\":\"%s\",\"args\":{}},\n",
					threadId, startTimeDiv1000, startTimeRem1000Str, newname);
			fprintf(gTimingFile, "{\"cat\":\"timing\",\"pid\":1,\"tid\":%d,\"ts\":%llu.%s ,\"ph\":\"E\",\"name\":\"%s\",\"args\":{}}",
					threadId, endTimeDiv1000, endTimeRem1000Str, newname);
#endif
#endif
		}
		m_numTimings = 0;
	}

	void addTiming(tukk name, i32 threadId, zu64 startTime, zu64 endTime)
	{
		if (m_numTimings >= DRX3D_TIMING_CAPACITY)
		{
			return;
		}

		if (m_timings[0].size() == 0)
		{
			m_timings[0].resize(DRX3D_TIMING_CAPACITY);
		}

		i32 slot = m_numTimings++;

		m_timings[m_activeBuffer][slot].m_name = name;
		m_timings[m_activeBuffer][slot].m_threadId = threadId;
		m_timings[m_activeBuffer][slot].m_usStartTime = startTime;
		m_timings[m_activeBuffer][slot].m_usEndTime = endTime;
	}

	i32 m_numTimings;
	i32 m_activeBuffer;
	AlignedObjectArray<Timing> m_timings[1];
};
//#ifndef DRX3D_NO_PROFILE
Timings gTimings[DRX3D_QUICKPROF_MAX_THREAD_COUNT];
#define MAX_NESTING 1024
i32 gStackDepths[DRX3D_QUICKPROF_MAX_THREAD_COUNT] = {0};
tukk gFuncNames[DRX3D_QUICKPROF_MAX_THREAD_COUNT][MAX_NESTING];
zu64 gStartTimes[DRX3D_QUICKPROF_MAX_THREAD_COUNT][MAX_NESTING];
//#endif

Clock clk;

bool gProfileDisabled = true;

void MyDummyEnterProfileZoneFunc(tukk msg)
{
}

void MyDummyLeaveProfileZoneFunc()
{
}

void MyEnterProfileZoneFunc(tukk msg)
{
	if (gProfileDisabled)
		return;

	i32 threadId = QuickprofGetCurrentThreadIndex2();
	if (threadId < 0 || threadId >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return;

	if (gStackDepths[threadId] >= MAX_NESTING)
	{
		Assert(0);
		return;
	}
	gFuncNames[threadId][gStackDepths[threadId]] = msg;
	gStartTimes[threadId][gStackDepths[threadId]] = clk.getTimeNanoseconds();
	if (gStartTimes[threadId][gStackDepths[threadId]] <= gStartTimes[threadId][gStackDepths[threadId] - 1])
	{
		gStartTimes[threadId][gStackDepths[threadId]] = 1 + gStartTimes[threadId][gStackDepths[threadId] - 1];
	}
	gStackDepths[threadId]++;

}
void MyLeaveProfileZoneFunc()
{
	if (gProfileDisabled)
		return;

	i32 threadId = QuickprofGetCurrentThreadIndex2();
	if (threadId < 0 || threadId >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return;

	if (gStackDepths[threadId] <= 0)
	{
		return;
	}

	gStackDepths[threadId]--;

	tukk name = gFuncNames[threadId][gStackDepths[threadId]];
	zu64 startTime = gStartTimes[threadId][gStackDepths[threadId]];

	zu64 endTime = clk.getTimeNanoseconds();
	gTimings[threadId].addTiming(name, threadId, startTime, endTime);

}

void b3ChromeUtilsStartTimings()
{
	m_firstTiming = true;
	gProfileDisabled = false;  //true;
	b3SetCustomEnterProfileZoneFunc(MyEnterProfileZoneFunc);
	b3SetCustomLeaveProfileZoneFunc(MyLeaveProfileZoneFunc);

	//also for drx3D 2.x API
	SetCustomEnterProfileZoneFunc(MyEnterProfileZoneFunc);
	SetCustomLeaveProfileZoneFunc(MyLeaveProfileZoneFunc);
}

void b3ChromeUtilsStopTimingsAndWriteJsonFile(tukk fileNamePrefix)
{
	b3SetCustomEnterProfileZoneFunc(MyDummyEnterProfileZoneFunc);
	b3SetCustomLeaveProfileZoneFunc(MyDummyLeaveProfileZoneFunc);
	//also for drx3D 2.x API
	SetCustomEnterProfileZoneFunc(MyDummyEnterProfileZoneFunc);
	SetCustomLeaveProfileZoneFunc(MyDummyLeaveProfileZoneFunc);
	char fileName[1024];
	static i32 fileCounter = 0;
	sprintf(fileName, "%s_%d.json", fileNamePrefix, fileCounter++);
	gTimingFile = fopen(fileName, "w");
	if (gTimingFile)
	{
		fprintf(gTimingFile, "{\"traceEvents\":[\n");
		//dump the content to file
		for (i32 i = 0; i < DRX3D_QUICKPROF_MAX_THREAD_COUNT; i++)
		{
			if (gTimings[i].m_numTimings)
			{
				printf("Writing %d timings for thread %d\n", gTimings[i].m_numTimings, i);
				gTimings[i].flush();
			}
		}
		fprintf(gTimingFile, "\n],\n\"displayTimeUnit\": \"ns\"}");
		fclose(gTimingFile);
	}
	else
	{
		drx3DPrintf("Error opening file");
		drx3DPrintf(fileName);
	}
	gTimingFile = 0;
}

void b3ChromeUtilsEnableProfiling()
{
	gProfileDisabled = false;
}
