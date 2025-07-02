// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "QToolWindowManagerCommon.h"
#include "IToolWindowDragHandler.h"
class QTOOLWINDOWMANAGER_EXPORT QToolWindowDragHandlerNinePatch :
	public IToolWindowDragHandler
{
public:
	void startDrag() Q_DECL_OVERRIDE;
	void switchedArea(IToolWindowArea* lastArea, IToolWindowArea* newArea) Q_DECL_OVERRIDE;
	QToolWindowAreaTarget getTargetFromPosition(IToolWindowArea* area) const Q_DECL_OVERRIDE;
	bool isHandlerWidget(QWidget* widget) const Q_DECL_OVERRIDE;
	QToolWindowAreaTarget finishDrag(QList<QWidget*> toolWindows, IToolWindowArea* source, IToolWindowArea* destination) Q_DECL_OVERRIDE;
	IToolWindowDragHandler::SplitSizes getSplitSizes(i32 originalSize) const Q_DECL_OVERRIDE;

	QRect getRectFromCursorPos(QWidget* previewArea, IToolWindowArea* area) const Q_DECL_OVERRIDE;

};


