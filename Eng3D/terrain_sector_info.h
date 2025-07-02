// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/terrain_sector.h>

STRUCT_INFO_BEGIN(STerrainNodeChunk)
STRUCT_VAR_INFO(nChunkVersion, TYPE_INFO(i16))
STRUCT_VAR_INFO(bHasHoles, TYPE_INFO(i16))
STRUCT_VAR_INFO(boxHeightmap, TYPE_INFO(AABB))
STRUCT_VAR_INFO(fOffset, TYPE_INFO(float))
STRUCT_VAR_INFO(fRange, TYPE_INFO(float))
STRUCT_VAR_INFO(nSize, TYPE_INFO(i32))
STRUCT_VAR_INFO(nSurfaceTypesNum, TYPE_INFO(i32))
STRUCT_INFO_END(STerrainNodeChunk)

STRUCT_INFO_BEGIN(SHeightMapItem)
STRUCT_BITFIELD_INFO(surface, TYPE_INFO(u32), 20)
STRUCT_BITFIELD_INFO(height, TYPE_INFO(u32), 12)
STRUCT_INFO_END(SHeightMapItem)
