// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/GpuParticleContainer.h>

namespace gpu_pfx2
{
void CParticleContainer::Initialize(bool isDoubleBuffered)
{
	m_defaultData.Initialize(isDoubleBuffered);
	m_counter.CreateDeviceBuffer();
}

void CParticleContainer::ReadbackCounter(u32 readLength)
{
	m_counter.Readback(readLength);
}

i32 CParticleContainer::RetrieveCounter(u32 readLength)
{
	i32 result = 0;
	if (const uint* counter = m_counter.Map(readLength))
	{
		result = counter[0];
		m_counter.Unmap();
	}
	return result;
}

}
