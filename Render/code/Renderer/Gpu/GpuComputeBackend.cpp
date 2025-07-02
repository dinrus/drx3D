// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/GpuComputeBackend.h>
#include <drx3D/Render/DriverD3D.h>

namespace gpu
{

CounterReadbackUsed::CounterReadbackUsed()
{
#if DRX_PLATFORM_DURANGO && defined(DEVICE_SUPPORTS_PERFORMANCE_DEVICE) && TODO
	// count buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.MiscFlags = 0;
	bufferDesc.ByteWidth = 16;
	bufferDesc.StructureByteStride = sizeof(u32);

	HRESULT hr = D3DAllocateGraphicsMemory(1 * sizeof(u32), 0, 0, D3D11_GRAPHICS_MEMORY_ACCESS_CPU_CACHE_COHERENT, &m_basePtr);
	gcpRendD3D.GetPerformanceDevice().CreatePlacementBuffer(&bufferDesc, m_basePtr, &m_countReadbackBuffer);
#else
	m_countReadbackBuffer = new CGpuBuffer();
	m_countReadbackBuffer->Create(1, sizeof(u32), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_CPU_READ, nullptr);
#endif

	m_readbackCalled = false;
}

i32 CounterReadbackUsed::Retrieve()
{
	if (!m_readbackCalled)
		return 0;

#if DRX_RENDERER_VULKAN
	GetDeviceObjectFactory().GetVKScheduler()->FlushToFence(m_readbackFence);
	GetDeviceObjectFactory().GetVKScheduler()->WaitForFence(m_readbackFence);
#endif

	i32 result = 0;
	CDeviceObjectFactory::DownloadContents<false>(m_countReadbackBuffer->GetDevBuffer()->GetBuffer(), 0, 0, sizeof(i32), D3D11_MAP_READ, &result);

	m_readbackCalled = false;

	return result;
}

void CounterReadbackUsed::Readback(CDeviceBuffer* pBuffer)
{
	if (!pBuffer)
		DrxFatalError("pBuffer is 0");

	CDeviceCommandListRef commandList = GetDeviceObjectFactory().GetCoreCommandList();
	commandList.GetCopyInterface()->Copy(pBuffer, m_countReadbackBuffer->GetDevBuffer());
	m_readbackCalled = true;

#if DRX_RENDERER_VULKAN
	m_readbackFence = GetDeviceObjectFactory().GetVKScheduler()->InsertFence();
#endif
}

DataReadbackUsed::DataReadbackUsed(u32 size, u32 stride) : m_readback(nullptr)
{
	// Create the Readback Buffer
	// This is used to read the results back to the CPU
#if DRX_PLATFORM_DURANGO && defined(DEVICE_SUPPORTS_PERFORMANCE_DEVICE) && TODO
	D3D11_BUFFER_DESC readback_buffer_desc;
	ZeroMemory(&readback_buffer_desc, sizeof(readback_buffer_desc));
	readback_buffer_desc.ByteWidth = size * stride;
	readback_buffer_desc.Usage = D3D11_USAGE_STAGING;
	readback_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	readback_buffer_desc.StructureByteStride = stride;

	HRESULT hr = D3DAllocateGraphicsMemory(size * stride, 0, 0, D3D11_GRAPHICS_MEMORY_ACCESS_CPU_CACHE_COHERENT, &m_basePtr);
	gcpRendD3D.GetPerformanceDevice().CreatePlacementBuffer(&readback_buffer_desc, m_basePtr, &m_readback);
#else
	m_readback = new CGpuBuffer();
	m_readback->Create(size, stride, DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_CPU_READ, nullptr);
#endif

	m_stride = stride;
	m_size = size;
	m_readbackCalled = false;
}

ukk DataReadbackUsed::Map(u32 readLength)
{
	if (!m_readbackCalled)
		return nullptr;

#if DRX_RENDERER_VULKAN
	GetDeviceObjectFactory().GetVKScheduler()->FlushToFence(m_readbackFence);
	GetDeviceObjectFactory().GetVKScheduler()->WaitForFence(m_readbackFence);
#endif

	// Readback the data
	return CDeviceObjectFactory::Map(m_readback->GetDevBuffer()->GetBuffer(), 0, 0, readLength * m_stride, D3D11_MAP_READ);
}

void DataReadbackUsed::Unmap()
{
	CDeviceObjectFactory::Unmap(m_readback->GetDevBuffer()->GetBuffer(), 0, 0, 0, D3D11_MAP_READ);
	m_readbackCalled = false;
}

void DataReadbackUsed::Readback(CGpuBuffer* buf, u32 readLength)
{
	const SResourceRegionMapping region =
	{
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ readLength * m_stride, 1, 1, 1 }
	};

	GetDeviceObjectFactory().GetCoreCommandList().GetCopyInterface()->Copy(buf->GetDevBuffer(), m_readback->GetDevBuffer(), region);
	
	m_readbackCalled = true;

#if DRX_RENDERER_VULKAN
	m_readbackFence = GetDeviceObjectFactory().GetVKScheduler()->InsertFence();
#endif
}
}
