// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   TabLineEdit.cpp
//  Version:     v1.00
//  Created:     03/03/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TabLineEdit.h"
#include <QEvent>
#include <QKeyEvent>

bool QTabLineEdit::event(QEvent* pEvent)
{
	if (pEvent->type() == QEvent::KeyPress)
	{
		QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
		if (pKeyEvent->key() == Qt::Key_Tab)
		{
			tabPressed();
			return true;
		}
	}
	return QLineEdit::event(pEvent);
}

