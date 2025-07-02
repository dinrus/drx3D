// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

#include "Terrain/Ui/GenerateTerrainTextureUi.h"

class CResizeTerrainTextureDialog
	: public CEditorDialog
{
public:
	typedef CGenerateTerrainTextureUi::Result Result;

	CResizeTerrainTextureDialog(const Result& initial);

	Result GetResult() const;

private:
	CGenerateTerrainTextureUi* m_pTexture;
};

