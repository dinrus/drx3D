// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "DockableDialog.h"

#include <QBoxLayout>
#include <QWindow>

#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>
#include "IEditorClassFactory.h"

CDockableDialog::CDockableDialog(const QString& dialogNameId, tukk paneClassId, bool saveSize /*= true*/)
	: QSandboxWindow(CEditorDialog::s_config)
	, m_dialogNameId(dialogNameId)
{
	IClassDesc* pClassDesc = GetIEditor()->GetClassFactory()->FindClass(paneClassId);
	DRX_ASSERT(pClassDesc && ESYSTEM_CLASS_VIEWPANE == pClassDesc->SystemClassID());

	IViewPaneClass* pViewPaneClass = static_cast<IViewPaneClass*>(pClassDesc);
	DRX_ASSERT(!pViewPaneClass->SinglePane());

	m_pane = pViewPaneClass->CreatePane();
	DRX_ASSERT(m_pane);

	QFrame* frame = new QFrame();
	auto mainLayout = new QVBoxLayout();
	mainLayout->setMargin(0);
	mainLayout->addWidget(m_pane->GetWidget());
	frame->setLayout(mainLayout);
	internalSetContents(frame);

	QVariant var = GetIEditor()->GetPersonalizationManager()->GetProperty(m_dialogNameId, "pane_layout");
	if (var.isValid())
		m_pane->SetState(var.toMap());

	QRect rect = m_pane->GetPaneRect();
	resize(rect.width(), rect.height());

	SetTitle(m_pane->GetPaneTitle());
}

CDockableDialog::~CDockableDialog()
{
	SaveState();
}

void CDockableDialog::SaveState()
{
	GetIEditor()->GetPersonalizationManager()->SetProperty(m_dialogNameId, "pane_layout", QVariant(m_pane->GetState()));
}

bool CDockableDialog::Execute()
{
	show();
	return true;
}

void CDockableDialog::Raise()
{
	if (window())
	{
		window()->raise();
		activateWindow();
	}
}

void CDockableDialog::Popup()
{
	show();
	Raise();
}

void CDockableDialog::Popup(const QPoint &position, const QSize& size)
{
	resize(size);
	Popup();
	if (window())
		window()->move(position);
}

void CDockableDialog::SetPosCascade()
{
	if (window())
	{
		static i32 i = 0;
		window()->move(QPoint(32, 32) * (i + 1));
		i = (i + 1) % 10;
	}
}

void CDockableDialog::SetHideOnClose()
{
	setAttribute(Qt::WA_DeleteOnClose, false);
}

void CDockableDialog::SetTitle(const QString& title)
{
	setWindowTitle(title);
}

void CDockableDialog::closeEvent(QCloseEvent* e)
{
	SaveState();
}

void CDockableDialog::hideEvent(QHideEvent* e)
{
	SaveState();
}

