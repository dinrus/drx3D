// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Range.h>
#include <drx3D/CoreX/xxhash.h>
#include <drx3D/Plugins/fasthash/fasthash.inl>
#include <drx3D/Plugins/concqueue/concqueue.hpp>

typedef u8  UINT8;
typedef u32   UINT32;
typedef unsigned short UINT16;

extern i32 g_nPrintDX11;

#define DX11_PTR(T)        _smart_ptr < T >
#define DX11_NEW_RAW(ctor) NDrxDX11::PassAddRef(new ctor)

#ifdef _DEBUG

    #define DX11_FUNC_LOG do {                        \
        if (g_nPrintDX11)                               \
        { DrxLog("DX11 function call: %s", __FUNC__); } \
    } while (0);

#else
    #define DX11_FUNC_LOG do {} while (0);
#endif

typedef struct D3D11_HEAP_PROPERTIES
{
    D3D11_USAGE Type;
    D3D11_CPU_ACCESS_FLAG CPUPageProperty;
}   D3D11_HEAP_PROPERTIES;

typedef struct D3D11_RESOURCE_DESC
{
    D3D11_RESOURCE_DIMENSION Dimension;
    UINT64 Alignment;
    UINT64 Width;
    UINT Height;
    UINT16 DepthOrArraySize;
    UINT16 MipLevels;
    DXGI_FORMAT Format;
    UINT StructureByteSize;
    DXGI_SAMPLE_DESC SampleDesc;
    D3D11_RESOURCE_MISC_FLAG Flags;
}   D3D11_RESOURCE_DESC;

#define DX11_RESOURCE_FLAG_HIFREQ_HEAP BIT(31)
#define DX11_CONCURRENCY_ANALYZER      false
#define DX11_FENCE_ANALYZER            false

// Extract lowest set isolated bit "intrinsic" -> _blsi_u32 (Jaguar == PS4|XO, PileDriver+, Haswell+)
#define blsi(field) (field & (-static_cast<INT>(field)))

namespace NDrxDX11
{

// Forward declarations
class CDevice;
class CResource;
class CView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
static T* PassAddRef(T* ptr)
{
    if (ptr)
    {
        ptr->AddRef();
    }

    return ptr;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
static T* PassAddRef(const _smart_ptr<T>& ptr)
{
    if (ptr)
    {
        ptr.get()->AddRef();
    }

    return ptr.get();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef uint32_t THash;
template<size_t length>
ILINE THash ComputeSmallHash(ukk data, UINT seed = 666)
{
    return fasthash::fasthash32<length>(data, seed);
}

//---------------------------------------------------------------------------------------------------------------------
UINT              GetDXGIFormatSize(DXGI_FORMAT format);

ILINE bool        IsDXGIFormatCompressed(DXGI_FORMAT format)
{
    return
      (format >= DXGI_FORMAT_BC1_TYPELESS  && format <= DXGI_FORMAT_BC5_SNORM) ||
      (format >= DXGI_FORMAT_BC6H_TYPELESS && format <= DXGI_FORMAT_BC7_UNORM_SRGB);
}

ILINE bool IsDXGIFormatCompressed4bpp(DXGI_FORMAT format)
{
    return
      (format >= DXGI_FORMAT_BC1_TYPELESS && format <= DXGI_FORMAT_BC1_UNORM_SRGB) ||
      (format >= DXGI_FORMAT_BC4_TYPELESS && format <= DXGI_FORMAT_BC4_SNORM);
}

ILINE bool IsDXGIFormatCompressed8bpp(DXGI_FORMAT format)
{
    return
      (format >= DXGI_FORMAT_BC2_TYPELESS  && format <= DXGI_FORMAT_BC3_UNORM_SRGB) ||
      (format >= DXGI_FORMAT_BC5_TYPELESS  && format <= DXGI_FORMAT_BC5_SNORM) ||
      (format >= DXGI_FORMAT_BC6H_TYPELESS && format <= DXGI_FORMAT_BC7_UNORM_SRGB);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRefCounted
{
     i32 m_RefCount;

public:
    CRefCounted() : m_RefCount(0)
    {}

    CRefCounted(CRefCounted&& r) : m_RefCount(std::move(r.m_RefCount))
    {}

    CRefCounted& operator=(CRefCounted&& r)
    { m_RefCount = std::move(r.m_RefCount); return *this; }

    void AddRef() threadsafe
    {
        DrxInterlockedIncrement(&m_RefCount);
    }
    void Release() threadsafe
    {
        if (!DrxInterlockedDecrement(&m_RefCount))
        {
            delete this;
        }
    }
    void ReleaseNoDelete() threadsafe
    {
        DrxInterlockedDecrement(&m_RefCount);
    }

protected:
    virtual ~CRefCounted() {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDeviceObject : public CRefCounted
{
public:
    ILINE CDevice* GetDevice() const
    {
        return m_pDevice;
    }

protected:
    CDeviceObject(CDevice* device);
    virtual ~CDeviceObject();

    CDeviceObject(CDeviceObject&& r)
        : CRefCounted(std::move(r))
        , m_pDevice(std::move(r.m_pDevice))
    {}

    CDeviceObject& operator=(CDeviceObject&& r)
    {
        CRefCounted::operator=(std::move(r));

        m_pDevice = std::move(r.m_pDevice);

        return *this;
    }

    ILINE void SetDevice(CDevice* device)
    {
        m_pDevice = device;
    }

private:
    CDevice* m_pDevice;
};

static const UINT CONSTANT_BUFFER_ELEMENT_SIZE = 16U;
}

#include <drx3D/Render/D3D/DX11/DX11Device.hpp>
