// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>

class QDialogLineDatabaseModel;
class QAdvancedTreeView;
class QDeepFilterProxyModel;
class QLineEdit;

class CDialogLinesDatabaseImportExportHelper
{
public:
	bool ImportFromFile(tukk szSourceTsvFile);
	bool ExportToFile(tukk szTargetTsvFile);

protected:
	enum class eDataColumns
	{
		lineID = 0, //LineSet
		priority, //LineSet
		flags,  //LineSet
		maxQueueDuration, //LineSet
		text, //Line
		audioStart, //Line
		audioStop, //Line
		lipsyncAnimation, //Line
		standaloneFile, //Line
		pauseLength, //Line
		customData, //Line
		NUM_COLUMNS
	};

	bool SplitStringList(const wchar_t* szStringToSplit, i32 stringLength, const wchar_t delimeter1, const wchar_t delimeter2, std::vector<wstring>* outResult, bool bTrim, bool bBothDelimeter);

	std::vector<wstring> m_dataColumns = { L"lineID", L"priority", L"flags", L"maxQueueDuration", L"text", L"audioStart", L"audioStop", L"lipsyncAnimation", L"standaloneFile", L"pauseLength", L"customData" };
	std::vector<i32> m_dataColumnsIndex = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
};

class CDialogLinesEditorWidget : public QWidget
{
public:
	CDialogLinesEditorWidget(QWidget* pParent);

private:
	void   Save();
	string GetDataFolder();

	QAdvancedTreeView*        m_pTree;
	QDeepFilterProxyModel*    m_pFilterModel;
	QDialogLineDatabaseModel* m_pModel;
	QLineEdit*                m_pFilterLineEdit;
	CDialogLinesDatabaseImportExportHelper m_importExportHelper;
};

