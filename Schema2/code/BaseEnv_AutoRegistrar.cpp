// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>

namespace SchematycBaseEnv
{
	CAutoRegistrar::CAutoRegistrar(AutoRegistrarFunctionPtr pFunction)
		: m_pFunction(pFunction)
	{}

	void CAutoRegistrar::Process()
	{
		for(const CAutoRegistrar* pInstance = GetFirstInstance(); pInstance; pInstance = pInstance->GetNextInstance())
		{
			pInstance->m_pFunction();
		}
	}
}