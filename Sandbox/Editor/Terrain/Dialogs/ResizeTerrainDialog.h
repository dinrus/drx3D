// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

#include "Terrain/Ui/GenerateHeightmapUi.h"
#include "Terrain/Ui/GenerateTerrainTextureUi.h"

class CResizeTerrainDialog
	: public CEditorDialog
{
	Q_OBJECT

public:
	struct Result
	{
		CGenerateHeightmapUi::Result      heightmap;
		CGenerateTerrainTextureUi::Result texture;
	};

public:
	explicit CResizeTerrainDialog(const Result& initial);

	Result GetResult() const;

private:
	CGenerateHeightmapUi*      m_pHeightmap;
	CGenerateTerrainTextureUi* m_pTexture;
};

