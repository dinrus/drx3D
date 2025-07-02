// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _AUTODETECT_CPU_TESTSUITE_
#define _AUTODETECT_CPU_TESTSUITE_

#pragma once

#if DRX_PLATFORM_WINDOWS

class CCPUTestSuite
{
public:
	i32 RunTest();
};

#endif // #if DRX_PLATFORM_WINDOWS

#endif // #ifndef _AUTODETECT_CPU_TESTSUITE_
