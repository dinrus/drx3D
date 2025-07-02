// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DialogCommon.h"

// EditorCommon
#include <AssetSystem/AssetEditor.h>

#include <vector>

namespace MeshImporter
{

class CBaseDialog;

} //endns MeshImporter

class QWidget;

//! Main frame of the mesh importer.
class CEditorAdapter : public CAssetEditor, public MeshImporter::IDialogHost
{
public:
	CEditorAdapter(std::unique_ptr<MeshImporter::CBaseDialog> pDialog, QWidget* pParent = nullptr);

	// IDialogHost implementation.
	virtual void Host_AddMenu(tukk menu) override;
	virtual void Host_AddToMenu(tukk menu, tukk command) override;

	// CDockableEditor implementation.

	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override;

protected:

	// CAssetEditor implementation.
	virtual bool OnOpenAsset(CAsset* pAsset) override;
	virtual bool OnSaveAsset(CEditableAsset& editAsset) override;
	virtual bool OnAboutToCloseAsset(string& reason) const override;
	virtual void OnCloseAsset() override;

	// CEditor implementation.
	virtual bool OnSave() override;
	virtual bool OnSaveAs() override;

	virtual void customEvent(QEvent* pEvent) override;
private:
	std::unique_ptr<MeshImporter::CBaseDialog> m_pDialog;
};

