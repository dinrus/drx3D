// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QValidator>

namespace ACE
{
static QRegularExpression const s_regexInvalidFileName(QRegularExpression(R"([^<>:;,?"*|\\/]*)"));
static QRegularExpression const s_regexInvalidFilePath(QRegularExpression(R"([^<>:;,?"*|]*)"));

class CNameValidator final : public QRegularExpressionValidator
{
public:

	explicit CNameValidator(QRegularExpression const& regex, QWidget* const pParent);

	CNameValidator() = delete;

	// QRegularExpressionValidator
	virtual QValidator::State validate(QString& string, i32& pos) const override;
	virtual void fixup(QString& input) const override;
	// ~QRegularExpressionValidator

private:

	QWidget* const m_pParent;
	QString        m_toolTipText;
};
} //endns ACE
