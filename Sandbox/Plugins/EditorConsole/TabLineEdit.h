// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   TabLineEdit.h
//  Version:     v1.00
//  Created:     03/03/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <QLineEdit>

//subclassed QLineEdit that allows capturing of tab
class QTabLineEdit : public QLineEdit
{
	Q_OBJECT;

public:
	QTabLineEdit(QWidget* pWidget) : QLineEdit(pWidget) {}
	bool event(QEvent* pEvent);

signals:
	void tabPressed();
};

