// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

#include <QAbstractItemModel>

struct SPreferencePage;
class QObjectTreeWidget;
class QSplitter;
class QContainer;

class EDITOR_COMMON_API QPreferencePage : public QWidget
{
	Q_OBJECT
public:
	QPreferencePage(SPreferencePage* pPreferencePage, QWidget* pParent = nullptr);
	QPreferencePage(std::vector<SPreferencePage*> preferences, tukk path, QWidget* pParent = nullptr);

private slots:
	void OnPropertyChanged();
	void OnResetToDefault();

private:
	SPreferencePage* m_pPreferencePage;
	const string m_path;
};


class EDITOR_COMMON_API QPreferencesDialog : public CDockableWidget
{
public:
	QPreferencesDialog(QWidget* pParent = nullptr);
	~QPreferencesDialog();

	//////////////////////////////////////////////////////////
	// CDockableWidget implementation
	virtual tukk GetPaneTitle() const override { return "Preferences"; }
	virtual QRect       GetPaneRect() override        { return QRect(0, 0, 640, 480); }
	//////////////////////////////////////////////////////////

protected:
	void OnPreferencePageSelected(tukk path);
	void OnPreferencesReset();

private:
	QContainer*        m_pContainer;
	QSplitter*         m_pSplitter;
	QObjectTreeWidget* m_pTreeView;
	string             m_currPath;
};

