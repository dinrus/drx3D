// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QObject>
#include <drx3D/CoreX/Serialization/StringList.h>

// Private class, should not be used directly
class QWidget;
class QPropertyDialog;
class CBatchFileDialog : public QObject
{
	Q_OBJECT
public slots:
	void OnSelectAll();
	void OnSelectNone();
	void OnLoadList();
public:
	QPropertyDialog* m_dialog;
	struct SContent;
	SContent*        m_content;
};
// ^^^

struct SBatchFileSettings
{
	tukk               scanExtension;
	tukk               scanFolder;
	tukk               title;
	tukk               descriptionText;
	tukk               listLabel;
	tukk               stateFilename;
	bool                      useDrxPak;
	bool                      allowListLoading;
	Serialization::StringList explicitFileList;
	i32                       defaultWidth;
	i32                       defaultHeight;

	SBatchFileSettings()
		: useDrxPak(true)
		, allowListLoading(true)
		, descriptionText("Batch Selected Files")
		, listLabel("Files")
		, stateFilename("batchFileDialog.state")
		, title("Batch Files")
		, scanFolder("")
		, scanExtension("")
	{
	}
};

bool EDITOR_COMMON_API ShowBatchFileDialog(Serialization::StringList* filenames, const SBatchFileSettings& settings, QWidget* parent);

