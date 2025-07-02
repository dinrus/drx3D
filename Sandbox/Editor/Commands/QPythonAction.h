// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D, 2014.
// -------------------------------------------------------------------------
//  File name: QPythonAction.h
//  Created:   26/09/2014 by timur
//  Description: Special Qt Action derived class to bind with python calls
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QAction>

class QWidget;

//////////////////////////////////////////////////////////////////////////
class SANDBOX_API QPythonAction : public QAction
{
public:
	QPythonAction(const QString& actionName, const QString& actionText, QObject* parent, tukk pythonCall = 0);
	QPythonAction(const QString& actionName, tukk pythonCall, QObject* parent);
	~QPythonAction() {}

protected:
	void OnTriggered();
};

