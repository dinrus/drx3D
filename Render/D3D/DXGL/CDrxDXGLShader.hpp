// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLShader.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for D3D11 shader interfaces
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSHADER__
#define __DRXDXGLSHADER__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SShader;
}

class CDrxDXGLShader : public CDrxDXGLDeviceChild
{
public:
	CDrxDXGLShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLShader();

	NDrxOpenGL::SShader* GetGLShader();
private:
	_smart_ptr<NDrxOpenGL::SShader> m_spGLShader;
};

class CDrxDXGLVertexShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLVertexShader, D3D11VertexShader)

	CDrxDXGLVertexShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11VertexShader)
	}
};

class CDrxDXGLHullShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLHullShader, D3D11HullShader)

	CDrxDXGLHullShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11HullShader)
	}
};

class CDrxDXGLDomainShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDomainShader, D3D11DomainShader)

	CDrxDXGLDomainShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11DomainShader)
	}
};

class CDrxDXGLGeometryShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLGeometryShader, D3D11GeometryShader)

	CDrxDXGLGeometryShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11GeometryShader)
	}
};

class CDrxDXGLPixelShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLPixelShader, D3D11PixelShader)

	CDrxDXGLPixelShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11PixelShader)
	}
};

class CDrxDXGLComputeShader : public CDrxDXGLShader
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLComputeShader, D3D11ComputeShader)

	CDrxDXGLComputeShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice) : CDrxDXGLShader(pGLShader, pDevice)
	{
		DXGL_INITIALIZE_INTERFACE(D3D11ComputeShader)
	}
};

#endif  //__DRXDXGLSHADER__
