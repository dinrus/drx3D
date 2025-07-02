// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/Shape/PolylineTool.h"
#include "ArgumentModel.h"
#include "Core/Polygon.h"

namespace Designer
{
class PrimitiveShape
{
public:

	void CreateBox(const BrushVec3& mins, const BrushVec3& maxs, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateSphere(const BrushVec3& mins, const BrushVec3& maxs, i32 numSides, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateSphere(const BrushVec3& vCenter, float radius, i32 numSides, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateCylinder(const BrushVec3& mins, const BrushVec3& maxs, i32 numSides, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateCylinder(PolygonPtr pBaseDiscPolygon, float fHeight, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateCone(const BrushVec3& mins, const BrushVec3& maxs, i32 numSides, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateCone(PolygonPtr pBaseDiscPolygon, float fHeight, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateRectangle(const BrushVec3& mins, const BrushVec3& maxs, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;
	void CreateDisc(const BrushVec3& mins, const BrushVec3& maxs, i32 numSides, std::vector<PolygonPtr>* pOutPolygonList = NULL) const;

private:

	void CreateCircle(const BrushVec3& mins, const BrushVec3& maxs, i32 numSides, std::vector<BrushVec3>& outVertexList) const;
};
}

