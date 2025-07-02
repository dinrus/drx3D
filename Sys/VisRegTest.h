// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   VisRegTest.h
//  Version:     v1.00
//  Created:     14/07/2010 by Nicolas Schulz.
//  Описание: Visual Regression Test
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __VisRegTest_h__
#define __VisRegTest_h__
#pragma once

struct IConsoleCmdArgs;

class CVisRegTest
{
public:
	static u32k SampleCount = 16;
	static u32k MaxNumGPUTimes = 5;
	static const float  MaxStreamingWait;

protected:

	enum ECmd
	{
		eCMDStart,
		eCMDFinish,
		eCMDOnMapLoaded,
		eCMDConsoleCmd,
		eCMDLoadMap,
		eCMDWaitStreaming,
		eCMDWaitFrames,
		eCMDGoto,
		eCMDCaptureSample
	};

	struct SCmd
	{
		ECmd   cmd;
		u32 freq;
		string args;

		SCmd() {}
		SCmd(ECmd cmd, const string& args, u32 freq = 1)
		{ this->cmd = cmd; this->args = args; this->freq = freq; }
	};

	struct SSample
	{
		tukk imageName;
		float       frameTime;
		u32      drawCalls;
		float       gpuTimes[MaxNumGPUTimes];

		SSample() : imageName(NULL), frameTime(0.f), drawCalls(0)
		{
			for (u32 i = 0; i < MaxNumGPUTimes; ++i)
				gpuTimes[i] = 0;
		}
	};

protected:
	string               m_testName;
	std::vector<SCmd>    m_cmdBuf;
	std::vector<SSample> m_dataSamples;
	u32               m_nextCmd;
	u32               m_cmdFreq;
	i32                  m_waitFrames;
	float                m_streamingTimeout;
	i32                  m_curSample;
	bool                 m_quitAfterTests;

public:

	CVisRegTest();

	void Init(IConsoleCmdArgs* pParams);
	void AfterRender();

protected:

	bool LoadConfig(tukk configFile);
	void ExecCommands();
	void LoadMap(tukk mapName);
	void CaptureSample(const SCmd& cmd);
	void Finish();
	bool WriteResults();
};

#endif // __VisRegTest_h__
