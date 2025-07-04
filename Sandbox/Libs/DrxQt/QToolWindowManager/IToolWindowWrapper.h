// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <QWidget>
#include <QVariantMap>
#include "QToolWindowManagerCommon.h"

class QTOOLWINDOWMANAGER_EXPORT IToolWindowWrapper
{
public:
	virtual ~IToolWindowWrapper() {};
	virtual QWidget* getWidget() = 0;
	virtual QWidget* getContents() = 0;
	virtual void setContents(QWidget* widget) = 0;
	virtual void startDrag() = 0;
	virtual void hide() = 0;
	virtual void deferDeletion() = 0;
	virtual void setParent(QWidget* newParent) = 0;
};
Q_DECLARE_INTERFACE(IToolWindowWrapper, "QToolWindowManager/IToolWindowWrapper");
