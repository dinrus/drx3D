// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DX11Base.hpp>

i32 g_nPrintDX11 = 0;

namespace NDrxDX11
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
	DX11_LOG(g_nPrintDX11, "DX11 object destroyed: %p", this);
}

}
