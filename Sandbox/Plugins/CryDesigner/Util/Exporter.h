// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Core/Polygon.h"

class CBrushObject;
class CPakFile;
class CDrxMemFile;

namespace Designer
{
class AreaSolidObject;
class ClipVolumeObject;
struct ExportedBrushGeom
{
	enum EFlags
	{
		SUPPORT_LIGHTMAP = 0x01,
		NO_PHYSICS       = 0x02,
	};
	i32  size;
	char filename[128];
	i32  flags;
	Vec3 m_minBBox;
	Vec3 m_maxBBox;
};

struct ExportedBrushMaterial
{
	i32  size;
	char material[64];
};

class Exporter
{
public:
	void ExportBrushes(const string& path, CPakFile& pakFile);

private:
	bool ExportAreaSolid(const string& path, AreaSolidObject* pAreaSolid, CPakFile& pakFile) const;
	bool ExportClipVolume(const string& path, ClipVolumeObject* pClipVolume, CPakFile& pakFile);
	void ExportStatObj(const string& path, IStatObj* pStatObj, CBaseObject* pObj, i32 renderFlag, const string& sGeomFileName, CPakFile& pakFile);

	struct AreaSolidStatistic
	{
		i32 numOfClosedPolygons;
		i32 numOfOpenPolygons;
		i32 totalSize;
	};

	static void ComputeAreaSolidMemoryStatistic(AreaSolidObject* pAreaSolid, AreaSolidStatistic& outStatistic, std::vector<PolygonPtr>& optimizedPolygons);

	//////////////////////////////////////////////////////////////////////////
	std::map<CMaterial*, i32>          m_mtlMap;
	std::vector<ExportedBrushGeom>     m_geoms;
	std::vector<ExportedBrushMaterial> m_materials;
};
}

