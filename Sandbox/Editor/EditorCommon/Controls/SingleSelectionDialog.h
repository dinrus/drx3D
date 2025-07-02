// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/EditorDialog.h"

#include <drx3D/CoreX/String/DrxString.h>

#include <vector>

class CSingleSelectionDialog : public CEditorDialog
{
public:
	CSingleSelectionDialog(QWidget* pParent = nullptr);

	void SetOptions(const std::vector<string>& options);

	i32 GetSelectedIndex() const;

private:
	void Rebuild();

private:
	std::vector<string> m_options;
	i32 m_selectedOptionIndex;
};
