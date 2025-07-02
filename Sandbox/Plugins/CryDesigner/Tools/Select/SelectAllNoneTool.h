// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class Model;

class SelectAllNoneTool : public BaseTool
{
public:

	SelectAllNoneTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;

	static void SelectAllVertices(CBaseObject* pObject, Model* pModel);
	static void SelectAllEdges(CBaseObject* pObject, Model* pModel);
	static void SelectAllFaces(CBaseObject* pObject, Model* pModel);

	static void DeselectAllVertices();
	static void DeselectAllEdges();
	static void DeselectAllFaces();
};
}

