// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SplineObject.h"

class CSplineDistributor : public CSplineObject
{
protected:
	CSplineDistributor();

public:
	// from CSplineObject
	void OnUpdate() override;
	void SetLayerId(u16 nLayerId) override;

protected:
	DECLARE_DYNCREATE(CSplineDistributor)

	//overrided from CBaseObject.
	void       Done() override;
	bool       CreateGameObject() override;
	void       UpdateVisibility(bool visible) override;
	XmlNodeRef Export(const string& levelPath, XmlNodeRef& xmlNode) override;
	void       InvalidateTM(i32 nWhyFlags) override;
	void       DeleteThis() override                      { delete this; }
	// Ignore default draw highlight.
	void       DrawHighlight(DisplayContext& dc) override {}
	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	//! Called when variable changes.
	void OnParamChange(IVariable* pVariable);
	void OnGeometryChange(IVariable* pVariable);

	void FreeGameData();
	void LoadGeometry(const string& filename);
	void UpdateEngineNode(bool bOnlyTransform = false);

private:
	void SetObjectCount(i32 num);

protected:
	CVariable<string> mv_geometryFile;
	CVariable<float>   mv_step;

	CVariable<bool>    mv_follow;
	CVariable<float>   mv_zAngle;

	CVariable<bool>    mv_outdoor;
	CVariable<bool>    mv_castShadowMaps;
	CVariable<bool>    mv_rainOccluder;
	CVariable<bool>    mv_registerByBBox;
	CVariableEnum<i32> mv_hideable;
	CVariable<i32>     mv_ratioLOD;
	CVariable<i32>     mv_ratioViewDist;
	CVariable<bool>    mv_excludeFromTriangulation;
	// keep for future implementation if we need
	//CVariable<float> mv_aiRadius;
	CVariable<bool>           mv_noDecals;
	CVariable<bool>           mv_recvWind;
	CVariable<float>          mv_integrQuality;
	CVariable<bool>           mv_Occluder;

	_smart_ptr<CEdMesh>       m_pGeometry;
	std::vector<IRenderNode*> m_renderNodes;

	bool                      m_bGameObjectCreated;
};

