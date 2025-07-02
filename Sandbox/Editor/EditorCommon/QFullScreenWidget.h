// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QWidget.h>

class QEvent;
class QFullScreenWidgetWindow;

//! This is used to provide fullscreen functionnality to any widget, simply pass the widget you want to use fullscreen in the constructor.
class EDITOR_COMMON_API QFullScreenWidget : public QWidget
{
	Q_OBJECT
public:
	friend class QFullScreenWidgetWindow;

	QFullScreenWidget(QWidget* w);
	virtual ~QFullScreenWidget();

	void                            Restore();
	void                            ShowFullScreen();
	void                            focusInEvent(QFocusEvent* event) override;
	bool                            eventFilter(QObject* obj, QEvent* event) override;

	static QFullScreenWidgetWindow* GetCurrentFullScreenWindow();
	static bool                     IsFullScreenWindowActive();

	static QFullScreenWidget*       GetLastFullScreenWidget() { return s_lastfullScreenWidget; }

private:

	static QFullScreenWidgetWindow* s_fullScreenWindow;
	static QFullScreenWidget*       s_lastfullScreenWidget;

	QWidget*                        m_maximizable;
};

