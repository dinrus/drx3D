// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DX12SamplerState.hpp>

namespace NDrxDX12
{

//---------------------------------------------------------------------------------------------------------------------
CSamplerState::CSamplerState()
	: CRefCounted()
	, m_DescriptorHandle(INVALID_CPU_DESCRIPTOR_HANDLE)
{
	// clear before use
	memset(&m_unSamplerDesc, 0, sizeof(m_unSamplerDesc));
}

//---------------------------------------------------------------------------------------------------------------------
CSamplerState::~CSamplerState()
{

}

}
