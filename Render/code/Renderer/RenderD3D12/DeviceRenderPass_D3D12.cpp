// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#include <drx3D/Render/DeviceResourceSet_D3D12.h>	
#include <drx3D/Render/DeviceRenderPass_D3D12.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeviceRenderPass::UpdateImpl(const CDeviceRenderPassDesc& passDesc)
{
	if (!passDesc.GetDeviceRendertargetViews(m_RenderTargetViews, m_RenderTargetCount))
		return false;

	if (!passDesc.GetDeviceDepthstencilView(m_pDepthStencilView))
		return false;

	return true;
}
