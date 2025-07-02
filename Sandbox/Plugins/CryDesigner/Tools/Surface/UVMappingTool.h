// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/Select/SelectTool.h"
#include "Core/UVIsland.h"

struct ITransformManipulator;

namespace Designer
{
class Model;
class UVMappingTool;

enum ETexParamFlag
{
	eTexParam_Offset = 0x01,
	eTexParam_Scale  = 0x02,
	eTexParam_Rotate = 0x04,
	eTexParam_All    = 0xFF
};

struct UVMappingParameter
{
	Vec2           m_UVOffset;
	Vec2           m_ScaleOffset;
	float          m_Rotate;
	Vec2           m_TilingXY;

	UVMappingTool* m_pTool;

	UVMappingParameter() :
		m_UVOffset(Vec2(0, 0)),
		m_ScaleOffset(Vec2(1, 1)),
		m_Rotate(0),
		m_TilingXY(Vec2(1, 1)),
		m_pTool(NULL)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

class UVMappingTool : public SelectTool
{
public:

	UVMappingTool(EDesignerTool tool) : SelectTool(tool)
	{
		m_UVParameter.m_pTool = this;
		m_nPickFlag = ePF_Polygon;
	}

	void Enter() override;
	void Leave() override;

	bool OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	bool OnLButtonUp(CViewport* view, UINT nFlags, CPoint point) override;
	bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;
	void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	void ApplyTextureInfo(const TexInfo& texInfo, i32 nModifiedParts);
	void FitTexture(float fTileU, float fTileV);

	void SelectPolygonsBySubMatID(i32 matID);
	bool GetTexInfoOfSelectedPolygon(TexInfo& outTexInfo) const;

	void OnManipulatorDrag(IDisplayViewport* pView, ITransformManipulator* pManipulator, CPoint& p0, BrushVec3 value, i32 nFlags) override;
	void OnManipulatorBegin(IDisplayViewport* pView, ITransformManipulator* pManipulator, CPoint& point, i32 flags) override;
	void OnManipulatorEnd(IDisplayViewport* pView, ITransformManipulator* pManipulator) override;
	void RecordTextureMappingUndo(tukk sUndoDescription) const;

	void ShowGizmo();

	void OnFitTexture();
	void OnReset();
	void OnSelectPolygons();
	void OnAssignSubMatID();
	void OnCopy();
	void OnPaste();
	void OpenUVMappingWnd();

	void UpdatePanel(const TexInfo& texInfo);

	void Serialize(Serialization::IArchive& ar) { m_UVParameter.Serialize(ar); }
	void OnChangeParameter(bool continuous) override;

private:

	bool QueryPolygon(const BrushRay& ray, i32& nOutPolygonIndex, bool& bOutNew) const;
	void SetTexInfoToPolygon(PolygonPtr pPolygon, const TexInfo& texInfo, i32 nModifiedParts);
	void FillSecondShelfWithSelectedElements();

private:

	struct UVContext
	{
		void Init()
		{
			m_UVInfos.clear();
		}

		BrushVec3 m_MouseDownPos;
		std::vector<std::pair<PolygonPtr, TexInfo>> m_UVInfos;
	};
	UVContext          m_MouseDownContext;
	UVMappingParameter m_UVParameter;
	UVMappingParameter m_PrevUVParameter;
	UVMappingParameter m_CopiedUVMappingParameter;
};
}

