// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Renderer/IGpuParticles.h>
#include <drx3D/Render/D3D/Gpu/GpuComputeBackend.h>
#include <memory.h>

namespace gpu_pfx2
{

// this structure is mirrored on GPU
struct SDefaultParticleData
{
	Vec3   Position;
	Vec3   Velocity;
	u32 Color;
	u32 AuxData;
};

class CParticleContainer
{
public:
	CParticleContainer(i32 maxParticles) : m_defaultData(maxParticles), m_counter(4) {}
	void        Initialize(bool isDoubleBuffered);
	size_t      GetMaxParticles() const  { return m_defaultData.Get().GetSize(); }
	bool        HasDefaultParticleData() { return m_defaultData.Get().IsDeviceBufferAllocated(); }

	void        Clear()                  { m_defaultData.Reset(); }
	void        Swap()                   { m_defaultData.Swap(); }

	void ReadbackCounter(u32 readLength);
	i32 RetrieveCounter(u32 readLength);

	CGpuBuffer& GetDefaultParticleDataBuffer()     { return m_defaultData.Get().GetBuffer(); };
	CGpuBuffer& GetDefaultParticleDataBackBuffer() { return m_defaultData.GetBackBuffer().GetBuffer(); }

private:
	// this will only be double buffered when needed (i.e. when the particles get sorted)
	gpu::CDoubleBuffered<gpu::CStructuredResource<SDefaultParticleData, gpu::BufferFlagsReadWrite>> m_defaultData;
	gpu::CStructuredResource<uint, gpu::BufferFlagsReadWriteReadback>                               m_counter;
};
}
