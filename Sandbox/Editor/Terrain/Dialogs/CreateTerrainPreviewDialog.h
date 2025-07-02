// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

#include "Terrain/Ui/TerrainTextureDimensionsUi.h"

class CCreateTerrainPreviewDialog
	: public CEditorDialog
{
public:
	typedef CTerrainTextureDimensionsUi::Result Result;

	CCreateTerrainPreviewDialog(const Result& initial);

	Result GetResult() const;

private:
	CTerrainTextureDimensionsUi* m_pDimensions;
};

