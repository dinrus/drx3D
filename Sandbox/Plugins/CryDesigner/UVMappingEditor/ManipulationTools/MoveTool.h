// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SelectTool.h"
#include "../UVCluster.h"

namespace Designer {
namespace UVMapping
{
class MoveTool : public SelectTool
{
public:
	MoveTool(EUVMappingTool tool);

	void OnLButtonDown(const SMouseEvent& me) override;
	void OnLButtonUp(const SMouseEvent& me) override;
	void OnMouseMove(const SMouseEvent& me) override;

	void OnGizmoLMBDown(i32 mode) override;
	void OnGizmoLMBUp(i32 mode) override;
	void OnTransformGizmo(i32 mode, const Vec3& offset) override;

private:

	void RecordUndo();
	void EndUndoRecord();

	struct DraggingMove
	{
		bool         bStarted;
		i32          mouse_x, mouse_y;
		mutable bool bEnough;

		void         Start(i32 x, i32 y)
		{
			bStarted = true;
			mouse_x = x;
			mouse_y = y;
			bEnough = false;
		}

		bool IsDraggedEnough(i32 x, i32 y) const
		{
			if (!bEnough && (std::abs(mouse_x - x) > 5 || std::abs(mouse_y - y) > 5))
				bEnough = true;
			return bEnough;
		}
	};

	UVCluster    m_VertexCluster;
	DraggingMove m_DraggingContext;
	Vec3         m_PrevHitPos;
};

GENERATE_MOVETOOL_CLASS(Island, EUVMappingTool)
GENERATE_MOVETOOL_CLASS(Polygon, EUVMappingTool)
GENERATE_MOVETOOL_CLASS(Edge, EUVMappingTool)
GENERATE_MOVETOOL_CLASS(Vertex, EUVMappingTool)

}
}

