// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/TypedConstantBuffer.h>

//  предварительные объявления
class CShader;

namespace gpu
{

struct CounterReadbackEmpty
{
	i32 Readback(CDeviceBuffer* pBuffer)
	{
		assert(0);
		return 0;
	}
};

struct CounterReadbackUsed
{
	CounterReadbackUsed();
	void Readback(CDeviceBuffer* pBuffer);
	i32  Retrieve();

private:
	CGpuBuffer* m_countReadbackBuffer;
#ifdef DURANGO
	uk       m_basePtr;
#endif

#if DRX_RENDERER_VULKAN
	uint64      m_readbackFence;
#endif

	bool        m_readbackCalled;
};

struct DataReadbackEmpty
{
	DataReadbackEmpty(i32 size, i32 stride){};
	ukk Map()
	{
		assert(0);
		return NULL;
	};
	void Unmap() { assert(0); };
};

struct DataReadbackUsed
{
	DataReadbackUsed(u32 size, u32 stride);
	void Readback(CGpuBuffer* buf, u32 readLength);
	ukk Map(u32 readLength);
	void Unmap();

private:
	CGpuBuffer* m_readback;
#ifdef DURANGO
	uk       m_basePtr;
#endif

#if DRX_RENDERER_VULKAN
	uint64      m_readbackFence;
#endif

	u32      m_stride;
	u32      m_size;
	bool        m_readbackCalled;
};

struct HostDataEmpty
{
	void   Resize(size_t size) {};
	u8* Get()               { return nullptr; }
};

struct HostDataUsed
{
	void   Resize(size_t size) { m_data.resize(size); }
	u8* Get()               { return &m_data[0]; }
private:
	std::vector<u8> m_data;
};

struct BufferFlagsReadWrite
{
	enum
	{
		flags = CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS
	};
	typedef CounterReadbackEmpty CounterReadback;
	typedef DataReadbackEmpty    DataReadback;
	typedef HostDataEmpty        HostData;
};

struct BufferFlagsReadWriteReadback
{
	enum
	{
		flags = CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS | CDeviceObjectFactory::USAGE_UAV_OVERLAP
	};
	typedef CounterReadbackEmpty CounterReadback;
	typedef DataReadbackUsed     DataReadback;
	typedef HostDataEmpty        HostData;
};

struct BufferFlagsReadWriteAppend
{
	enum
	{
		flags = CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::BIND_UNORDERED_ACCESS | CDeviceObjectFactory::USAGE_UAV_COUNTER
	};
	typedef CounterReadbackUsed CounterReadback;
	typedef DataReadbackEmpty   DataReadback;
	typedef HostDataEmpty       HostData;
};

struct BufferFlagsDynamic
{
	enum
	{
		flags = CDeviceObjectFactory::BIND_SHADER_RESOURCE | CDeviceObjectFactory::USAGE_CPU_WRITE
	};
	typedef CounterReadbackEmpty CounterReadback;
	typedef DataReadbackEmpty    DataReadback;
	typedef HostDataUsed         HostData;
};

template<typename BFlags> class CStridedResource
{
public:
	CStridedResource(i32 stride, i32 size)
		: m_size(size), m_stride(stride), m_dataReadback(size, stride)
	{
		m_buffer.Create(size, stride, DXGI_FORMAT_UNKNOWN, BFlags::flags, NULL);
	}
#if 0
	ID3D11UnorderedAccessView* GetUAV()    { return m_buffer.GetDeviceUAV(); };
	ID3D11ShaderResourceView*  GetSRV()    { return m_buffer.GetSRV(); };
	ID3D11Buffer*              GetBuffer() { return m_buffer.GetBuffer(); };
#endif

	void                       UpdateBufferContent(uk pData, size_t nSize)
	{
		m_buffer.UpdateBufferContent(pData, m_stride * nSize);
	};
	void        ReadbackCounter() { return m_counterReadback.Readback(m_buffer.GetDevBuffer()); };
	i32         RetrieveCounter() { return m_counterReadback.Retrieve(); };
	void        Readback()        { return m_dataReadback.Readback(m_buffer.GetDevBuffer()); };
	ukk Map()             { return (ukk )m_dataReadback.Map(); };
	void        Unmap()           { return m_dataReadback.Unmap(); };

private:
	i32k  m_size;
	i32k  m_stride;
	CGpuBuffer m_buffer;
	// this is only an nonempty struct if it is used
	typedef typename BFlags::CounterReadback CounterReadback;
	CounterReadback m_counterReadback;
	typedef typename BFlags::DataReadback    DataReadback;
	DataReadback    m_dataReadback;
};

template<typename T> inline DXGI_FORMAT DXGI_FORMAT_DETECT() { return DXGI_FORMAT_UNKNOWN; }
template<> inline DXGI_FORMAT DXGI_FORMAT_DETECT<i32>() { return DXGI_FORMAT_R32_SINT; }
template<> inline DXGI_FORMAT DXGI_FORMAT_DETECT<uint>() { return DXGI_FORMAT_R32_UINT; }
template<> inline DXGI_FORMAT DXGI_FORMAT_DETECT<float>() { return DXGI_FORMAT_R32_FLOAT; }

template<typename T> inline CDeviceObjectFactory::EResourceAllocationFlags USAGE_DETECT() { return CDeviceObjectFactory::USAGE_STRUCTURED; }
template<> inline CDeviceObjectFactory::EResourceAllocationFlags USAGE_DETECT<i32>() { return CDeviceObjectFactory::EResourceAllocationFlags(0); }
template<> inline CDeviceObjectFactory::EResourceAllocationFlags USAGE_DETECT<uint>() { return CDeviceObjectFactory::EResourceAllocationFlags(0); }
template<> inline CDeviceObjectFactory::EResourceAllocationFlags USAGE_DETECT<float>() { return CDeviceObjectFactory::EResourceAllocationFlags(0); }

template<typename T, typename BFlags> class CTypedResource
{
public:
	CTypedResource(i32 size) : m_size(size), m_dataReadback(size, sizeof(T))
	{
		m_hostData.Resize(size * sizeof(T));
	}

	void CreateDeviceBuffer()
	{
		m_buffer.Create(m_size, sizeof(T), DXGI_FORMAT_DETECT<T>(), USAGE_DETECT<T>() | BFlags::flags, NULL);
	}
	void FreeDeviceBuffer()
	{
		m_buffer.Release();
	}
	CGpuBuffer& GetBuffer() { return m_buffer; };

	T&          operator[](size_t i)
	{
		return *reinterpret_cast<T*>(m_hostData.Get() + i * sizeof(T));
	}
	size_t GetSize() const { return m_size; }
	void   UploadHostData()
	{
		UpdateBufferContent(m_hostData.Get(), m_size);
	};
	void UpdateBufferContent(ukk pData, size_t nSize)
	{
		m_buffer.UpdateBufferContent(pData, sizeof(T) * nSize);
	};
	void UpdateBufferContentAligned(ukk pData, size_t nSize)
	{
		m_buffer.UpdateBufferContent(pData, Align(sizeof(T) * nSize, DRX_PLATFORM_ALIGNMENT));
	};
	void     ReadbackCounter() { return m_counterReadback.Readback(m_buffer.GetDevBuffer()); };
	i32      RetrieveCounter() { return m_counterReadback.Retrieve(); };
	void     Readback(u32 readLength) { return m_dataReadback.Readback(&m_buffer, readLength); };
	const T* Map(u32 readLength) { return (const T*)m_dataReadback.Map(readLength); };
	void     Unmap() { return m_dataReadback.Unmap(); };
	bool     IsDeviceBufferAllocated() { return m_buffer.GetDevBuffer() != nullptr; }
private:
	i32k  m_size;
	CGpuBuffer m_buffer;
	// this is only an nonempty struct if it is used
	typedef typename BFlags::CounterReadback CounterReadback;
	CounterReadback m_counterReadback;
	typedef typename BFlags::DataReadback    DataReadback;
	DataReadback    m_dataReadback;
	typedef typename BFlags::HostData        HostData;
	HostData        m_hostData;
};

template<typename T, typename BFlags> using CStructuredResource = CTypedResource<T, BFlags>;

template<typename T> class CTypedConstantBuffer : public ::CTypedConstantBuffer<T, 256>
{
	typedef typename ::CTypedConstantBuffer<T, 256> TBase;
public:
	bool IsDeviceBufferAllocated() { return TBase::m_constantBuffer != nullptr; }
	T&   operator=(const T& hostData)
	{
		return TBase::m_hostBuffer = hostData;
	}
	T&       GetHostData()       { return TBase::m_hostBuffer; }
	const T& GetHostData() const { return TBase::m_hostBuffer; }
};

inline i32 GetNumberOfBlocksForArbitaryNumberOfThreads(i32k threads, i32k blockSize)
{
	return threads / blockSize + (threads % blockSize != 0);
}

template<class T>
class CDoubleBuffered
{
public:
	CDoubleBuffered(i32 size) : m_size(size), m_current(0), m_isDoubleBuffered(false) {}
	void Initialize(bool isDoubleBuffered)
	{
		Reset();

		m_buffers[0] = std::unique_ptr<T>(new T(m_size));
		m_buffers[0]->CreateDeviceBuffer();

		if (isDoubleBuffered)
		{
			m_buffers[1] = std::unique_ptr<T>(new T(m_size));
			m_buffers[1]->CreateDeviceBuffer();
		}

		m_isDoubleBuffered = isDoubleBuffered;
	}
	void Reset()
	{
		m_current = 0;
		m_buffers[0].reset();
		m_buffers[1].reset();
	}
	T&       Get()                 { return *m_buffers[m_current]; }
	const T& Get() const           { return *m_buffers[m_current]; }
	T&       GetBackBuffer()       { return *m_buffers[!m_current]; }
	const T& GetBackBuffer() const { return *m_buffers[!m_current]; }
	void     Swap()
	{
		if (m_isDoubleBuffered)
			m_current = !m_current;
	}
	i32 GetCurrentBufferId() const { return m_current; }

private:
	std::unique_ptr<T> m_buffers[2];
	i32k          m_size;
	i32                m_current;
	bool               m_isDoubleBuffered;
};

}
