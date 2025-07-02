// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "EditorFramework/Editor.h"
#include <QStringList>
#include <QEvent>
#include <QResizeEvent>
#include "SubstanceCommon.h"

class CAssetFoldersView;
class CFileNameLineEdit;
class CAsset;
class QScrollableBox;
class QDialogButtonBox;

namespace EditorSubstance
{
	namespace OutputEditor {
		class CSubstanceOutputEditorWidget;
	}

	class CProjectDefaultsPresetsEditor : public CDockableEditor
	{
		Q_OBJECT
	public:
		CProjectDefaultsPresetsEditor(QWidget* parent = nullptr);
		bool OnSave() final;
	protected:

		virtual tukk GetEditorName() const override { return "Substance Graph Default Mapping Editor"; };
		bool TryClose();

		virtual bool OnClose() override;

		virtual void closeEvent(QCloseEvent *) override;

		virtual bool CanQuit(std::vector<string>& unsavedChanges) override;

	private:
		OutputEditor::CSubstanceOutputEditorWidget* m_editorWidget;
		bool m_modified;
	};

}
