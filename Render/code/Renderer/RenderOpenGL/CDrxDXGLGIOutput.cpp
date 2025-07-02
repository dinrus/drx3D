// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIOutput.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for IDXGIOutput
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLGIOutput.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>
#include <drx3D/CoreX/String/UnicodeFunctions.h>

CDrxDXGLGIOutput::CDrxDXGLGIOutput(NDrxOpenGL::SOutput* pGLOutput)
	: m_spGLOutput(pGLOutput)
{
	DXGL_INITIALIZE_INTERFACE(DXGIOutput)
}

CDrxDXGLGIOutput::~CDrxDXGLGIOutput()
{
}

bool CDrxDXGLGIOutput::Initialize()
{
	ZeroMemory(&m_kDesc, sizeof(m_kDesc));

	Unicode::Convert(m_kDesc.DeviceName, m_spGLOutput->m_strDeviceName);

	if (m_spGLOutput->m_kModes.empty())
	{
		DXGL_ERROR("GL Output has no display modes");
		return false;
	}

	m_kDisplayModes.resize(m_spGLOutput->m_kModes.size());

	std::vector<NDrxOpenGL::SDisplayMode>::const_iterator kGLModeIter(m_spGLOutput->m_kModes.begin());
	const std::vector<NDrxOpenGL::SDisplayMode>::const_iterator kGLModeEnd(m_spGLOutput->m_kModes.end());
	std::vector<DXGI_MODE_DESC>::iterator kModeIter;
	for (kModeIter = m_kDisplayModes.begin(); kGLModeIter != kGLModeEnd; ++kGLModeIter, ++kModeIter)
	{
		NDrxOpenGL::GetDXGIModeDesc(&*kModeIter, *kGLModeIter);
	}

	return true;
}

NDrxOpenGL::SOutput* CDrxDXGLGIOutput::GetGLOutput()
{
	return m_spGLOutput;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIOutput implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIOutput::GetDesc(DXGI_OUTPUT_DESC* pDesc)
{
	*pDesc = m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLGIOutput::GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT* pNumModes, DXGI_MODE_DESC* pDesc)
{
	DXGL_TODO("Take into account Flags as well (for filtering scaled/interlaced modes) if required")
	Flags;

	DXGI_MODE_DESC* pDescEnd(pDesc == NULL ? NULL : pDesc + *pNumModes);
	std::vector<DXGI_MODE_DESC>::const_iterator kModeIter(m_kDisplayModes.begin());
	const std::vector<DXGI_MODE_DESC>::const_iterator kModeEnd(m_kDisplayModes.end());
	for (; kModeIter != kModeEnd; ++kModeIter)
	{
		if (EnumFormat == DXGI_FORMAT_UNKNOWN || EnumFormat == kModeIter->Format)
		{
			if (pDesc == NULL)
				++(*pNumModes);
			else if (pDesc < pDescEnd)
				*(pDesc++) = *kModeIter;
			else
				return DXGI_ERROR_MORE_DATA;
		}
	}

	return S_OK;
}

HRESULT CDrxDXGLGIOutput::FindClosestMatchingMode(const DXGI_MODE_DESC* pModeToMatch, DXGI_MODE_DESC* pClosestMatch, IUnknown* pConcernedDevice)
{
	struct SRank
	{
		u32 m_uOrdering;
		u32 m_uScaling;
		u32 m_uFormat;
		u32 m_uResolution;
		u32 m_uRefreshRate;

		SRank(u32 uDefaultValue)
			: m_uOrdering(uDefaultValue)
			, m_uScaling(uDefaultValue)
			, m_uFormat(uDefaultValue)
			, m_uResolution(uDefaultValue)
			, m_uRefreshRate(uDefaultValue)
		{
		}

		SRank(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			m_uOrdering = MatchOrdering(kDesc, kReference);
			m_uScaling = MatchScaling(kDesc, kReference);
			m_uFormat = kDesc.Format == kReference.Format;
			m_uResolution = abs((i32)kDesc.Width * (i32)kDesc.Height - (i32)kReference.Width * (i32)kReference.Height);
			m_uRefreshRate = (u32)abs((float)kDesc.RefreshRate.Numerator / (float)kDesc.RefreshRate.Denominator - (float)kReference.RefreshRate.Numerator / (float)kReference.RefreshRate.Denominator);
		}

		static bool MatchScaling(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			return kReference.Scaling != DXGI_MODE_SCALING_UNSPECIFIED && kReference.Scaling != kDesc.Scaling;
		}

		static bool MatchOrdering(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			return kReference.ScanlineOrdering != DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED && kReference.ScanlineOrdering != kDesc.ScanlineOrdering;
		}

		static bool MatchFormat(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			return kReference.Format == kDesc.Format;
		}

		static bool MatchResolution(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			return kDesc.Height >= kReference.Height && kDesc.Width >= kReference.Width;
		}

		static bool MatchRefreshRate(const DXGI_MODE_DESC& kDesc, const DXGI_MODE_DESC& kReference)
		{
			return kDesc.RefreshRate.Numerator * kReference.RefreshRate.Denominator >= kReference.RefreshRate.Numerator * kDesc.RefreshRate.Denominator;
		}

		bool operator<(const SRank& kOther) const
		{
			if (m_uOrdering != kOther.m_uOrdering)
				return m_uOrdering < kOther.m_uOrdering;
			if (m_uScaling != kOther.m_uScaling)
				return m_uScaling < kOther.m_uScaling;
			if (m_uFormat != kOther.m_uFormat)
				return m_uFormat < kOther.m_uFormat;
			if (m_uResolution != kOther.m_uResolution)
				return m_uResolution < kOther.m_uResolution;
			return m_uRefreshRate < kOther.m_uRefreshRate;
		}
	};

	bool bScanlineOrdering(pModeToMatch->ScanlineOrdering != DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED);
	bool bScaling(pModeToMatch->Scaling != DXGI_MODE_SCALING_UNSPECIFIED);
	bool bFormat(pModeToMatch->Format != DXGI_FORMAT_UNKNOWN);
	bool bResolution(pModeToMatch->Width != 0 || pModeToMatch->Height != 0);
	bool bRefreshRate(pModeToMatch->RefreshRate.Numerator != 0 || pModeToMatch->RefreshRate.Denominator != 0);

	if ((!bFormat && pConcernedDevice == NULL) ||
	    bResolution != (pModeToMatch->Width != 0 && pModeToMatch->Height != 0) ||
	    bRefreshRate != (pModeToMatch->RefreshRate.Numerator != 0 && pModeToMatch->RefreshRate.Denominator != 0))
	{
		return E_FAIL;
	}

	ID3D11Device* pConcernedD3D11Device(NULL);
	DXGI_MODE_DESC kTarget(*pModeToMatch);
	if (!bScanlineOrdering)
		kTarget.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	if (!bScaling)
		kTarget.Scaling = DXGI_MODE_SCALING_STRETCHED;
	if (!bFormat || !bResolution || !bRefreshRate)
	{
		DXGI_MODE_DESC kDXGIDesktopModeDesc;
		GetDXGIModeDesc(&kDXGIDesktopModeDesc, m_spGLOutput->m_kDesktopMode);

		if (!bFormat)
		{
			uk pvConcernedD3D11Device(NULL);
			if (pConcernedDevice != NULL && !FAILED(pConcernedDevice->QueryInterface(__uuidof(ID3D11Device), &pvConcernedD3D11Device)))
				pConcernedD3D11Device = static_cast<ID3D11Device*>(pvConcernedD3D11Device);
			else
				kTarget.Format = kDXGIDesktopModeDesc.Format;
		}

		if (!bResolution)
		{
			kTarget.Width = kDXGIDesktopModeDesc.Width;
			kTarget.Height = kDXGIDesktopModeDesc.Height;
		}

		if (!bRefreshRate)
		{
			kTarget.RefreshRate.Numerator = kDXGIDesktopModeDesc.RefreshRate.Numerator;
			kTarget.RefreshRate.Denominator = kDXGIDesktopModeDesc.RefreshRate.Denominator;
		}
	}

	u32 uMinMode = m_kDisplayModes.size();
	SRank kMinRank(~0);
	for (u32 uMode = 0; uMode < m_kDisplayModes.size(); ++uMode)
	{
		const DXGI_MODE_DESC& kMode(m_kDisplayModes.at(uMode));

		if ((bScanlineOrdering && !SRank::MatchOrdering(kMode, kTarget)) ||
		    (bScaling && !SRank::MatchScaling(kMode, kTarget)) ||
		    (bFormat && !SRank::MatchFormat(kMode, kTarget)) ||
		    (bResolution && !SRank::MatchResolution(kMode, kTarget)) ||
		    (bRefreshRate && !SRank::MatchRefreshRate(kMode, kTarget)))
			continue;

		if (pConcernedD3D11Device != NULL)
		{
			UINT uModeFormatSuppport;
			if (FAILED(pConcernedD3D11Device->CheckFormatSupport(kMode.Format, &uModeFormatSuppport)) ||
			    (uModeFormatSuppport & D3D11_FORMAT_SUPPORT_DISPLAY) == 0)
				continue;
		}

		SRank kModeRank(kMode, kTarget);
		if (kModeRank < kMinRank)
		{
			uMinMode = uMode;
			kMinRank = kModeRank;
		}
	}

	if (uMinMode < m_kDisplayModes.size())
	{
		*pClosestMatch = m_kDisplayModes.at(uMinMode);
		if (pClosestMatch->Scaling == DXGI_MODE_SCALING_UNSPECIFIED)
			pClosestMatch->Scaling = kTarget.Scaling;
		if (pClosestMatch->ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED)
			pClosestMatch->ScanlineOrdering = kTarget.ScanlineOrdering;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::WaitForVBlank(void)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::TakeOwnership(IUnknown* pDevice, BOOL Exclusive)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

void CDrxDXGLGIOutput::ReleaseOwnership(void)
{
	DXGL_NOT_IMPLEMENTED
}

HRESULT CDrxDXGLGIOutput::GetGammaControlCapabilities(DXGI_GAMMA_CONTROL_CAPABILITIES* pGammaCaps)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::SetGammaControl(const DXGI_GAMMA_CONTROL* pArray)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::GetGammaControl(DXGI_GAMMA_CONTROL* pArray)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::SetDisplaySurface(IDXGISurface* pScanoutSurface)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::GetDisplaySurfaceData(IDXGISurface* pDestination)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLGIOutput::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}
