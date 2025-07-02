// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/DrxTypeInfo.h>
//#include "../drx3D/DinrusX3dEng/CGF/ChunkFile.h"
#include <drx3D/Eng3D/CGFContent.h>

class CChunkData;
struct SVClothInfoCGF;

class CSaverVCloth
{
public:
	CSaverVCloth(CChunkData& chunkData, const SVClothInfoCGF* pVClothInfo, bool swapEndian);

	void WriteChunkHeader();
	void WriteChunkVertices();
	void WriteTriangleData();
	void WriteNndcNotAttachedOrdered();
	void WriteLinks();

private:

	CChunkData* m_pChunkData;
	const SVClothInfoCGF* m_pVClothInfo;
	bool m_bSwapEndian;

};

