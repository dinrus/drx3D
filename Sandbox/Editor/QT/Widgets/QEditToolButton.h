// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __QEditToolButton_h__
#define __QEditToolButton_h__

#pragma once
#include <QToolButton>

//////////////////////////////////////////////////////////////////////////
//! This is meant to be used for a button that will activate a CEditTool
//! The button will deactivate when the tool gets deactivated
class QEditToolButton : public QToolButton, public IEditorNotifyListener
{
	Q_OBJECT
public:
	QEditToolButton(QWidget* parent);
	~QEditToolButton();

	void SetToolClass(CRuntimeClass* toolClass, tukk userDataKey = 0, uk userData = 0);
	void SetToolName(const string& sEditToolName, tukk userDataKey = 0, uk userData = 0);
	// Set if this tool button relies on a loaded level / ready document. By default every tool button only works if a level is loaded.
	// However some tools are also used without a loaded level (e.g. UI Emulator)
	void SetNeedDocument(bool bNeedDocument) { m_bNeedDocument = bNeedDocument; }
	void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

protected Q_SLOTS:
	void OnClicked(bool bChecked);

private:
	void DetermineCheckedState();
	//! Tool associated with this button.
	CRuntimeClass* m_toolClass;
	string         m_userDataKey;
	bool           m_bNeedDocument;
	bool           m_ignoreClick;
	uk          m_userData;
};

//////////////////////////////////////////////////////////////////////////
class QEditToolButtonPanel : public QWidget
{
public:
	struct SButtonInfo
	{
		string         name;
		string         toolClassName;
		string         toolUserDataKey;
		string         toolUserData;
		string         toolTip;
		string         icon;
		bool           bNeedDocument;
		CRuntimeClass* pToolClass;

		SButtonInfo() : pToolClass(0), bNeedDocument(true) {};
	};

	enum class LayoutType
	{
		Grid,
		Vertical,
		Horizontal
	};

	QEditToolButtonPanel(LayoutType layoutType = LayoutType::Grid, QWidget* pParent = nullptr);
	virtual ~QEditToolButtonPanel();

	void AddButton(const SButtonInfo& button);
	void AddButton(string name, string toolClass);
	void AddButton(string name, CRuntimeClass* pToolClass);
	void EnableButton(string buttonName, bool disable);
	void ClearButtons();

	void UncheckAll();

protected:
	void ReleaseGuiButtons();

	//////////////////////////////////////////////////////////////////////////
	struct SButton
	{
		SButtonInfo      info;
		QEditToolButton* pButton;
		SButton() : pButton(0) {};
	};

	std::vector<SButton> m_buttons;
	LayoutType           m_layoutType;
};

#endif // __QEditToolButton_h__

