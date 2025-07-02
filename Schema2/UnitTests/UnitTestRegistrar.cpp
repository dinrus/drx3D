
#include  <drx3D/Schema2/StdAfx.h>
#include  <drx3D/Schema2/UnitTests/UnitTestRegistrar.h>

#include  <drx3D/Schema2/ILog.h>

namespace sxema2
{
	CUnitTestRegistrar::CUnitTestRegistrar(UnitTestFunctionPtr pFuntion, tukk szName)
		: m_pFuntion(pFuntion)
		, m_szName(szName)
	{}

	void CUnitTestRegistrar::RunUnitTests()
	{
		EUnitTestResultFlags allResultFlags = EUnitTestResultFlags::Success;
		for(const CUnitTestRegistrar* pInstance = GetFirstInstance(); pInstance; pInstance = pInstance->GetNextInstance())
		{
			const EUnitTestResultFlags testResultFlags = pInstance->m_pFuntion();
			if(testResultFlags != EUnitTestResultFlags::Success)
			{
				SXEMA2_SYSTEM_ERROR("Failed unit test '%s'!", pInstance->m_szName);
			}
			allResultFlags |= testResultFlags;
		}

		if((allResultFlags & EUnitTestResultFlags::FatalError) != 0)
		{
			SXEMA2_SYSTEM_FATAL_ERROR("Failed one or more unit tests!");
		}
		else if((allResultFlags & EUnitTestResultFlags::CriticalError) != 0)
		{
			SXEMA2_SYSTEM_CRITICAL_ERROR("Failed one or more unit tests!");
		}
	}
}
