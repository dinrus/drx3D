// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../BaseTool.h"
#include "../UVElement.h"

namespace Designer {
namespace UVMapping
{
struct UVElement;

class LoopSelectionTool : public BaseTool
{
public:
	LoopSelectionTool(EUVMappingTool tool) : BaseTool(tool) {}

	void        Enter() override;

	static bool BorderSelection(UVElementSetPtr pUVElements = NULL);
	static bool LoopSelection(UVElementSetPtr pUVElements = NULL);

private:
	static std::vector<UVElement> BorderSelection(const std::vector<UVElement>& input);
	static i32                    FindBestBorderUVEdge(const Vec2& uv0, const Vec2& uv1, UVIslandPtr pUVIslandPtr, const std::vector<UVEdge>& candidateUVEdges);

	static std::vector<UVElement> LoopSelection(const std::vector<UVElement>& input);
	static i32                    FindBestLoopUVEdge(const Vec2& uv0, const Vec2& uv1, UVIslandPtr pUVIslandPtr, const std::vector<UVEdge>& candidateUVEdges);
};
}
}

