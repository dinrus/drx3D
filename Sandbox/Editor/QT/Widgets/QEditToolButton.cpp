// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include "QEditToolButton.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "RecursionLoopGuard.h"
#include "DrxIcon.h"

//////////////////////////////////////////////////////////////////////////
QEditToolButton::QEditToolButton(QWidget* parent)
	: QToolButton(parent)
	, m_ignoreClick(false)
{
	m_toolClass = 0;
	//m_bNeedDocument = true;
	m_bNeedDocument = false;

	setCheckable(true);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(OnClicked(bool)));
	GetIEditorImpl()->RegisterNotifyListener(this);
}

QEditToolButton::~QEditToolButton()
{
	GetIEditorImpl()->UnregisterNotifyListener(this);
}

//////////////////////////////////////////////////////////////////////////
void QEditToolButton::SetToolName(const string& sEditToolName, tukk userDataKey, uk userData)
{
	IClassDesc* pClass = GetIEditorImpl()->GetClassFactory()->FindClass(sEditToolName);
	if (!pClass)
	{
		Warning("Editor Tool %s not registered.", (tukk)sEditToolName);
		return;
	}
	if (pClass->SystemClassID() != ESYSTEM_CLASS_EDITTOOL)
	{
		Warning("Class name %s is not a valid Edit Tool class.", (tukk)sEditToolName);
		return;
	}
	CRuntimeClass* pRtClass = pClass->GetRuntimeClass();
	if (!pRtClass || !pRtClass->IsDerivedFrom(RUNTIME_CLASS(CEditTool)))
	{
		Warning("Class name %s is not a valid Edit Tool class.", (tukk)sEditToolName);
		return;
	}
	m_toolClass = pRtClass;

	m_userData = userData;
	if (userDataKey)
		m_userDataKey = userDataKey;
}

//////////////////////////////////////////////////////////////////////////
void QEditToolButton::SetToolClass(CRuntimeClass* toolClass, tukk userDataKey, uk userData)
{
	m_toolClass = toolClass;

	m_userData = userData;
	if (userDataKey)
		m_userDataKey = userDataKey;

	// Now we can determine if we can be checked or not, since we have a valid tool class
	DetermineCheckedState();
}

/////////////////////////////////////////////////////////////////////////////
// QEditToolButton message handlers
void QEditToolButton::OnEditorNotifyEvent(EEditorNotifyEvent event)
{
	if (event == eNotify_OnEditToolEndChange)
	{
		DetermineCheckedState();
	}
}
void QEditToolButton::DetermineCheckedState()
{
	RECURSION_GUARD(m_ignoreClick);

	if (!m_toolClass)
	{
		if (isChecked())
			setChecked(false);
		return;
	}

	// Check tool state.
	CEditTool* tool = GetIEditorImpl()->GetEditTool();
	CRuntimeClass* toolClass = 0;
	if (tool)
		toolClass = tool->GetRuntimeClass();

	if (toolClass != m_toolClass)
	{
		if (isChecked())
			setChecked(false);
	}
	else
	{
		if (!isChecked())
			setChecked(true);
	}
}

//////////////////////////////////////////////////////////////////////////
void QEditToolButton::OnClicked(bool bChecked)
{
	RECURSION_GUARD(m_ignoreClick);

	if (!m_toolClass || m_bNeedDocument && !GetIEditorImpl()->IsDocumentReady())
	{
		if (isChecked())
			setChecked(false);
		return;
	}

	CEditTool* tool = GetIEditorImpl()->GetEditTool();
	if (!bChecked)
	{
		if (tool && tool->GetRuntimeClass() == m_toolClass)
		{
			GetIEditorImpl()->SetEditTool(0);
		}
		return;
	}
	else
	{
		CEditTool* pNewTool = (CEditTool*)m_toolClass->CreateObject();
		if (!pNewTool)
			return;

		if (m_userData)
			pNewTool->SetUserData(m_userDataKey, m_userData);

		// Must be last function, can delete this.
		GetIEditorImpl()->SetEditTool(pNewTool);
	}
}

//////////////////////////////////////////////////////////////////////////
QEditToolButtonPanel::QEditToolButtonPanel(LayoutType layoutType, QWidget* pParent)
	: QWidget(pParent)
	, m_layoutType(layoutType)
{
	QLayout* pLayout = nullptr;
	switch (m_layoutType)
	{
	case LayoutType::Grid:
		pLayout = new QGridLayout(this);
		break;

	case LayoutType::Vertical:
		pLayout = new QVBoxLayout(this);
		break;

	case LayoutType::Horizontal:
		pLayout = new QHBoxLayout(this);
		break;
	}

	pLayout->setSpacing(3);
	pLayout->setContentsMargins(3, 4, 4, 6);
	pLayout->setSizeConstraint(QLayout::SetNoConstraint);
}

QEditToolButtonPanel::~QEditToolButtonPanel()
{
	ClearButtons();
}

//////////////////////////////////////////////////////////////////////////
void QEditToolButtonPanel::AddButton(const SButtonInfo& button)
{
	SButton b;
	b.info = button;
	m_buttons.push_back(b);

	b.pButton = new QEditToolButton(this);
	b.pButton->setText(button.name.c_str());
	b.pButton->setToolTip(button.toolTip.c_str());
	b.pButton->setIcon(DrxIcon(button.icon.c_str()));
	b.pButton->setIconSize(QSize(24, 24));
	b.pButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	b.pButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	b.pButton->SetNeedDocument(b.info.bNeedDocument);

	if (m_layoutType == LayoutType::Grid)
	{
		i32 index = m_buttons.size() - 1;
		QGridLayout* grid = (QGridLayout*)layout();
		i32 column = index % 2;
		i32 row = index / 2;
		grid->addWidget(b.pButton, row, column);
	}
	else
	{
		layout()->addWidget(b.pButton);
	}

	if (b.info.pToolClass)
	{
		b.pButton->SetToolClass(b.info.pToolClass, b.info.toolUserDataKey, (uk )(tukk)b.info.toolUserData);
	}
	else if (!b.info.toolClassName.empty())
	{
		b.pButton->SetToolName(b.info.toolClassName, b.info.toolUserDataKey, (uk )(tukk)b.info.toolUserData);
	}
}

//////////////////////////////////////////////////////////////////////////
void QEditToolButtonPanel::AddButton(string name, string toolClass)
{
	SButtonInfo bi;
	bi.name = name;
	bi.toolClassName = toolClass;
	AddButton(bi);
}
//////////////////////////////////////////////////////////////////////////
void QEditToolButtonPanel::AddButton(string name, CRuntimeClass* pToolClass)
{
	SButtonInfo bi;
	bi.name = name;
	bi.pToolClass = pToolClass;
	AddButton(bi);
}
//////////////////////////////////////////////////////////////////////////
void QEditToolButtonPanel::ClearButtons()
{
	for (i32 i = 0; i < m_buttons.size(); i++)
	{
		delete m_buttons[i].pButton;
		m_buttons[i].pButton = 0;
	}
	m_buttons.clear();
}

void QEditToolButtonPanel::UncheckAll()
{
	for (i32 i = 0; i < m_buttons.size(); i++)
	{
		m_buttons[i].pButton->setChecked(false);
	}
}

void QEditToolButtonPanel::EnableButton(string buttonName, bool enable)
{
	for (i32 i = 0; i < m_buttons.size(); i++)
	{
		SButton& b = m_buttons[i];

		if (strcmp(b.info.name, buttonName) == 0 && b.pButton)
			b.pButton->setEnabled(enable);
	}
}

