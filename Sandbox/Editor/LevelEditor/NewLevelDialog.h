// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

#include "LevelAssetType.h"

#include <memory>

class CNewLevelDialog
	: public CEditorDialog
{
public:
	explicit CNewLevelDialog();
	~CNewLevelDialog();

	CLevelType::SCreateParams GetResult() const;

private:
	struct Implementation;
	std::unique_ptr<Implementation> p;
};

