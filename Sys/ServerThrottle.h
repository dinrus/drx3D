// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: System.cpp
//  Описание: Handle raising/lowering the frame rate on server
//               based upon CPU usage
//
//	История:
//	-May 28,2007: Originally Created by Craig Tiller
//
//////////////////////////////////////////////////////////////////////

#ifndef __SERVERTHROTTLE_H__
#define __SERVERTHROTTLE_H__

#pragma once

struct ISystem;

class CCPUMonitor;

class CServerThrottle
{
public:
	CServerThrottle(ISystem* pSys, i32 nCPUs);
	~CServerThrottle();
	void Update();

private:
	std::unique_ptr<CCPUMonitor> m_pCPUMonitor;

	void SetStep(i32 step, float* dueToCPU);

	float  m_minFPS;
	float  m_maxFPS;
	i32    m_nSteps;
	i32    m_nCurStep;
	ICVar* m_pDedicatedMaxRate;
	ICVar* m_pDedicatedCPU;
	ICVar* m_pDedicatedCPUVariance;
};

#endif
