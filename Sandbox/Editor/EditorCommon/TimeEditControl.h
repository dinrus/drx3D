// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QValidator>
#include <QLineEdit>

class CTimeValidator : public QValidator
{
public:
	explicit CTimeValidator(QWidget* parent = 0);

	virtual State validate(QString& input, i32& position) const;

	virtual void  fixup(QString& input) const;
};

class EDITOR_COMMON_API CTimeEditControl : public QLineEdit
{
	Q_OBJECT
public:
	explicit CTimeEditControl(QWidget* parent = 0);

	QTime time() const;
	void  setTime(const QTime& time);

signals:
	void         timeChanged(const QTime& time);
protected:
	static QTime TimeFromString(const QString& string);

	virtual void keyPressEvent(QKeyEvent* event);
};

