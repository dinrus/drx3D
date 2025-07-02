// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   GeomCacheMeshUpr.h
//  Created:     18/1/2013 by Axel Gneiting
//  Описание: Manages static meshes for geometry caches
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _GEOMCACHE_MESHMANAGER_
#define _GEOMCACHE_MESHMANAGER_

#pragma once

#if defined(USE_GEOM_CACHES)

	#include <drx3D/CoreX/Renderer/IRenderMesh.h>
	#include <drx3D/Eng3D/GeomCacheFileFormat.h>
	#include <drx3D/Eng3D/GeomCache.h>
	#include <drx3D/CoreX/StlUtils.h>

class CGeomCacheMeshUpr
{
public:
	void                    Reset();

	bool                    ReadMeshStaticData(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo, SGeomCacheStaticMeshData& staticMeshData) const;
	_smart_ptr<IRenderMesh> ConstructStaticRenderMesh(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo,
	                                                  SGeomCacheStaticMeshData& staticMeshData, tukk pFileName);
	_smart_ptr<IRenderMesh> GetStaticRenderMesh(const uint64 hash) const;
	void                    RemoveReference(SGeomCacheStaticMeshData& staticMeshData);
private:
	bool                    ReadMeshIndices(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo,
	                                        SGeomCacheStaticMeshData& staticMeshData, std::vector<vtx_idx>& indices) const;
	bool                    ReadMeshPositions(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo, strided_pointer<Vec3> positions) const;
	bool                    ReadMeshTexcoords(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo, strided_pointer<Vec2> texcoords) const;
	bool                    ReadMeshQTangents(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo, strided_pointer<SPipTangents> tangents) const;
	bool                    ReadMeshColors(CGeomCacheStreamReader& reader, const GeomCacheFile::SMeshInfo& meshInfo, strided_pointer<UCol> colors) const;

	struct SMeshMapInfo
	{
		_smart_ptr<IRenderMesh> m_pRenderMesh;
		uint                    m_refCount;
	};

	// Map from mesh hash to render mesh
	typedef std::unordered_map<uint64, SMeshMapInfo> TMeshMap;
	TMeshMap m_meshMap;
};

#endif
#endif
