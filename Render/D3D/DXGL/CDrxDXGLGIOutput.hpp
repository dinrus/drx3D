// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIOutput.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for IDXGIOutput
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLGIOUTPUT__
#define __DRXDXGLGIOUTPUT__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>

namespace NDrxOpenGL
{
struct SOutput;
}

class CDrxDXGLGIOutput : public CDrxDXGLGIObject
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLGIOutput, DXGIOutput)

	CDrxDXGLGIOutput(NDrxOpenGL::SOutput* pGLOutput);
	virtual ~CDrxDXGLGIOutput();

	bool                 Initialize();
	NDrxOpenGL::SOutput* GetGLOutput();

	// IDXGIOutput implementation
	HRESULT GetDesc(DXGI_OUTPUT_DESC* pDesc);
	HRESULT GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT* pNumModes, DXGI_MODE_DESC* pDesc);
	HRESULT FindClosestMatchingMode(const DXGI_MODE_DESC* pModeToMatch, DXGI_MODE_DESC* pClosestMatch, IUnknown* pConcernedDevice);
	HRESULT WaitForVBlank(void);
	HRESULT TakeOwnership(IUnknown* pDevice, BOOL Exclusive);
	void    ReleaseOwnership(void);
	HRESULT GetGammaControlCapabilities(DXGI_GAMMA_CONTROL_CAPABILITIES* pGammaCaps);
	HRESULT SetGammaControl(const DXGI_GAMMA_CONTROL* pArray);
	HRESULT GetGammaControl(DXGI_GAMMA_CONTROL* pArray);
	HRESULT SetDisplaySurface(IDXGISurface* pScanoutSurface);
	HRESULT GetDisplaySurfaceData(IDXGISurface* pDestination);
	HRESULT GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats);

protected:
	_smart_ptr<NDrxOpenGL::SOutput> m_spGLOutput;
	std::vector<DXGI_MODE_DESC>     m_kDisplayModes;
	DXGI_OUTPUT_DESC                m_kDesc;
};

#endif //__DRXDXGLGIOUTPUT__
