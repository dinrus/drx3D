// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <drx3D/Render/StdAfx.h>

#include <drx3D/Render/DeviceCommandListCommon.h>

void CDeviceCommandList::Close()
{
#if 0
	ERenderListID renderList = EFSLIST_INVALID; // TODO: set to correct renderlist
	bool bAsynchronous = false; // TODO set correct asynchronous mode

	gcpRendD3D->AddRecordedProfilingStats(EndProfilingSection(), renderList, bAsynchronous)
#endif

	CloseImpl();
}
