// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Sampler.h
//  Version:     v1.00
//  Created:     14/3/2005 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Sampler_h__
#define __Sampler_h__
#pragma once

#if DRX_PLATFORM_WINDOWS

class CSamplingThread;

// Symbol database, used Microsoft DebugAPI.
class CSymbolDatabase
{
public:
	CSymbolDatabase();
	~CSymbolDatabase();

	bool Init();

	// Lookup name of the function from instruction pointer.
	bool LookupFunctionName(uint64 ip, string& funcName);
	bool LookupFunctionName(uint64 ip, string& funcName, string& fileName, i32& lineNumber);

private:
	bool m_bInitialized;
};

//////////////////////////////////////////////////////////////////////////
// Sampler class is running a second thread which is at regular intervals
// eg 1ms samples main thread and stores current IP in the samples buffers.
// After sampling finishes it can resolve collected IP buffer info to
// the function names and calculated where most of the execution time spent.
//////////////////////////////////////////////////////////////////////////
class CSampler
{
public:
	struct SFunctionSample
	{
		string function;
		u32 nSamples; // Number of samples per function.
	};

	CSampler();
	~CSampler();

	void Start();
	void Stop();
	void Update();

	// Adds a new sample to the ip buffer, return false if no more samples can be added.
	bool AddSample(uint64 ip);
	void SetMaxSamples(i32 nMaxSamples);

	i32  GetSamplePeriod() const     { return m_samplePeriodMs; }
	void SetSamplePeriod(i32 millis) { m_samplePeriodMs = millis; }

private:
	void ProcessSampledData();
	void LogSampledData();

	// Buffer for IP samples.
	std::vector<uint64>          m_rawSamples;
	std::vector<SFunctionSample> m_functionSamples;
	i32                          m_nMaxSamples;
	bool                         m_bSampling;
	bool                         m_bSamplingFinished;

	i32                          m_samplePeriodMs;

	CSamplingThread*             m_pSamplingThread;
	CSymbolDatabase*             m_pSymDB;
};

#else // DRX_PLATFORM_WINDOWS

// Dummy sampler.
class CSampler
{
public:
	void Start()                        {}
	void Stop()                         {}
	void Update()                       {}
	void SetMaxSamples(i32 nMaxSamples) {}
	void SetSamplePeriod(i32 millis)    {}
};

#endif // DRX_PLATFORM_WINDOWS

#endif // __Sampler_h__
