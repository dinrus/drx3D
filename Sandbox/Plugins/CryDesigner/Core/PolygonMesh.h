// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Polygon.h"

namespace Designer
{
class PolygonMesh : public _i_reference_target_t
{
public:

	PolygonMesh();
	~PolygonMesh();

	void         SetPolygon(PolygonPtr pPolygon, bool bForce, const Matrix34& worldTM = Matrix34::CreateIdentity(), i32 dwRndFlags = 0, i32 nViewDistRatio = 100, i32 nMinSpec = 0, u8 materialLayerMask = 0);
	void         SetPolygons(const std::vector<PolygonPtr>& polygonList, bool bForce, const Matrix34& worldTM = Matrix34::CreateIdentity(), i32 dwRndFlags = 0, i32 nViewDistRatio = 100, i32 nMinSpec = 0, u8 materialLayerMask = 0);
	void         SetWorldTM(const Matrix34& worldTM);
	void         SetMaterial(IMaterial* pMaterial);
	void         ReleaseResources();
	IRenderNode* GetRenderNode() const { return m_pRenderNode; }

private:

	void ApplyMaterial();
	void UpdateStatObjAndRenderNode(const FlexibleMesh& mesh, const Matrix34& worldTM, i32 dwRndFlags, i32 nViewDistRatio, i32 nMinSpec, u8 materialLayerMask);
	void ReleaseRenderNode();
	void CreateRenderNode();

	std::vector<PolygonPtr> m_pPolygons;
	IStatObj*               m_pStatObj;
	IRenderNode*            m_pRenderNode;
	_smart_ptr<IMaterial>   m_pMaterial;

};
}

