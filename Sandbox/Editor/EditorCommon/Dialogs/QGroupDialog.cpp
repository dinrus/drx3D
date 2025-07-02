// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"

#include "QGroupDialog.h"
#include <QLineEdit>
#include <QVBoxLayout>

QGroupDialog::QGroupDialog(const QString& title, QWidget* pParent /*= NULL*/)
	: QStringDialog(title, pParent)
{
	m_pGroup = new QLineEdit(this);

	((QVBoxLayout*)layout())->insertWidget(0, new QLabel("Name"));
	((QVBoxLayout*)layout())->insertWidget(0, m_pGroup);
	((QVBoxLayout*)layout())->insertWidget(0, new QLabel("Group"));

	m_pGroup->setFocus();
}

void QGroupDialog::SetGroup(tukk str)
{
	m_pGroup->setText(str);
}

const string QGroupDialog::GetGroup()
{
	return m_pGroup->text().toUtf8().constData();
}


