// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "Core/Polygon.h"

namespace Designer
{
class Model;
class ModelDB;

class SmoothingGroup : public _i_reference_target_t
{
public:

	SmoothingGroup();
	SmoothingGroup(const std::vector<PolygonPtr>& polygons);

	~SmoothingGroup();

	void                    Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bUndo, Model* pModel);

	void                    SetPolygons(const std::vector<PolygonPtr>& polygons);
	void                    AddPolygon(PolygonPtr pPolygon);
	bool                    HasPolygon(PolygonPtr pPolygon) const;

	i32                     GetPolygonCount() const;
	PolygonPtr              GetPolygon(i32 nIndex) const;
	std::vector<PolygonPtr> GetAll() const;
	void                    RemovePolygon(PolygonPtr pPolygon);
	void                    Invalidate() { m_bValidmesh[0] = m_bValidmesh[1] = false; }

	const FlexibleMesh&     GetFlexibleMesh(Model* pModel, bool bGenerateBackFaces = false);

private:

	bool CalculateNormal(const BrushVec3& vPos, BrushVec3& vOutNormal) const;
	void UpdateMesh(Model* pModel, bool bGenerateBackFaces = false);

private:
	std::vector<PolygonPtr>  m_Polygons;
	std::set<PolygonPtr>     m_PolygonSet;
	std::unique_ptr<ModelDB> m_ModelDB;
	FlexibleMesh             m_FlexibleMesh;
	bool                     m_bValidmesh[2]; // 0 - Front Faces, 1 - Back Faces

};

typedef _smart_ptr<SmoothingGroup> SmoothingGroupPtr;
}

