// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/UnitTestRegistrar.h>

#include <drx3D/Schema/ILog.h>

namespace sxema
{
CUnitTestRegistrar::CUnitTestRegistrar(UnitTestFunctionPtr pFuntion, tukk szName)
	: m_pFuntion(pFuntion)
	, m_szName(szName)
{}

void CUnitTestRegistrar::RunUnitTests()
{
	UnitTestResultFlags allResultFlags = EUnitTestResultFlags::Success;
	for (const CUnitTestRegistrar* pInstance = GetFirstInstance(); pInstance; pInstance = pInstance->GetNextInstance())
	{
		const UnitTestResultFlags testResultFlags = pInstance->m_pFuntion();
		if (testResultFlags != EUnitTestResultFlags::Success)
		{
			SXEMA_CORE_ERROR("Failed unit test '%s'!", pInstance->m_szName);
		}
		allResultFlags.Add(testResultFlags);
	}

	if (allResultFlags.Check(EUnitTestResultFlags::FatalError))
	{
		SXEMA_CORE_FATAL_ERROR("Failed one or more unit tests!");
	}
	else if (allResultFlags.Check(EUnitTestResultFlags::CriticalError))
	{
		SXEMA_CORE_CRITICAL_ERROR("Failed one or more unit tests!");
	}
}
}
