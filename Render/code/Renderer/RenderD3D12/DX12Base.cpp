// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DX12Base.hpp>

i32 g_nPrintDX12 = 0;

namespace NDrxDX12
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
CDeviceObject::CDeviceObject(CDevice* device)
	: CRefCounted()
	, m_pDevice(device)
{

}

//---------------------------------------------------------------------------------------------------------------------
CDeviceObject::~CDeviceObject()
{
	DX12_LOG(g_nPrintDX12, "DX12 object destroyed: %p", this);
}

}
