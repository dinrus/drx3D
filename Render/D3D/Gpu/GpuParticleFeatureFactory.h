// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     09/04/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Render/GpuParticleFeatureBase.h>

namespace gpu_pfx2
{

template<class T>
CFeature* CreateInterfaceFunction() { return new T(); }

class CGpuInterfaceFactory
{
private:
	typedef CFeature* (* FactoryFuncPtr)();
	FactoryFuncPtr m_functions[eGpuFeatureType_COUNT];

public:
	CFeature* CreateInstance(EGpuFeatureType type)
	{
		return m_functions[static_cast<size_t>(type)]();
	}
	template<class T> void RegisterClass()
	{
		m_functions[static_cast<size_t>(T::type)] = &CreateInterfaceFunction<T>;
	}
	CGpuInterfaceFactory();
};
}
