// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "NameValidator.h"

#include <QToolTip>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CNameValidator::CNameValidator(QRegularExpression const& regex, QWidget* const pParent)
	: QRegularExpressionValidator(regex, pParent)
	, m_pParent(pParent)
{
	if (regex == s_regexInvalidFilePath)
	{
		m_toolTipText = "A path can't contain any of the following characters:\n: ; , * ? \" < > |";
	}
	else
	{
		m_toolTipText = "A name can't contain any of the following characters:\n\\ / : ; , * ? \" < > |";
	}
}

//////////////////////////////////////////////////////////////////////////
QValidator::State CNameValidator::validate(QString& string, i32& pos) const
{
	QValidator::State const state = QRegularExpressionValidator::validate(string, pos);

	if (state != QValidator::State::Acceptable)
	{
		QToolTip::showText(m_pParent->mapToGlobal(QPoint(0, -55)), m_toolTipText);
	}

	return state;
}

//////////////////////////////////////////////////////////////////////////
void CNameValidator::fixup(QString& input) const
{
	while (input.startsWith(".") || input.startsWith(" "))
	{
		input.remove(0, 1);
	}

	while (input.endsWith(".") || input.endsWith(" "))
	{
		input.remove(-1, 1);
	}
}
} //endns ACE
