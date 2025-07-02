// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "RasterMPMan.h"

CRasterMPMan::SMPData CRasterMPMan::scMPData[scMaxMatChunks][2];
HANDLE CRasterMPMan::scEventID;
HANDLE CRasterMPMan::scWorkerEventID;
u32 CRasterMPMan::scMPDataCount[2];
u32 CRasterMPMan::scActiveSet;

void CRasterMPMan::StartProjectTrianglesOS()
{
	if (m_CoreCount <= 1)
	{
		for (i32 i = 0; i < scMPDataCount[scActiveSet]; ++i)
		{
			SMPData& rMPData = scMPData[i][scActiveSet];
			rMPData.pBBoxRaster->ProjectTrianglesOS
			(
			  rMPData.cWorldMin,
			  rMPData.cWorldMax,
			  *rMPData.cpOSToWorldRotMatrix,
			  *rMPData.cpChunks,
			  rMPData.useConservativeFill,
			  rMPData.cpTriangleValidator
			);
		}
		scActiveSet = (scActiveSet + 1) & 0x1;//switch set
		scMPDataCount[scActiveSet] = 0;//reset it
	}
	else
	{
		if (m_ThreadActive)
			WaitForSingleObject(scEventID, INFINITE);//wait til last call was finished
		scActiveSet = (scActiveSet + 1) & 0x1;//switch set
		m_ThreadActive = true;
		scMPDataCount[scActiveSet] = 0;//reset it
		SetEvent(scWorkerEventID);//signal next processing
	}
}

#if defined(DO_RASTER_MP)
void CRasterMPMan::ThreadEntry()
{
	while (true)
	{
		WaitForSingleObject(scWorkerEventID, INFINITE);
		const bool cSetToProcess = (scActiveSet + 1) & 0x1;
		if (scMPDataCount[cSetToProcess] == 0xFFFFFFFF)//signaled thread exiting
		{
			SetEvent(scEventID);
			return;
		}
		for (i32 i = 0; i < scMPDataCount[cSetToProcess]; ++i)
		{
			SMPData& rMPData = scMPData[i][cSetToProcess];
			rMPData.pBBoxRaster->ProjectTrianglesOS
			(
			  rMPData.cWorldMin,
			  rMPData.cWorldMax,
			  *rMPData.cpOSToWorldRotMatrix,
			  *rMPData.cpChunks,
			  rMPData.useConservativeFill,
			  rMPData.cpTriangleValidator
			);
		}
		SetEvent(scEventID);
	}
}
#endif

