// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/EditorDialog.h>

#include <QWidget>

class CAssetImportContext;
class CAssetType;

//! Dialog that shows "Import" and "Import all" buttons, and the content of the drop importer.
class CAssetImportDialog : public CEditorDialog
{
public:
	CAssetImportDialog(
		CAssetImportContext& context,
		const std::vector<CAssetType*>& assetTypes,
		const std::vector<bool>& assetTypeSelection,
		const std::vector<string>& allFilePaths,
		QWidget* pParent = nullptr);

	std::vector<bool> GetAssetTypeSelection() const;

private:
	QWidget* CreateContentWidget(const std::vector<CAssetType*>& assetTypes);

private:
	CAssetImportContext& m_context;

	std::vector<bool> m_assetTypeSelection;
};

