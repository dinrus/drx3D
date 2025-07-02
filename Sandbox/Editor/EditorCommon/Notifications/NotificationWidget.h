// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

// Qt
#include <QWidget>

// DrxCommon
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class QLabel;
class QProgressBar;
class QTimer;

//! Used as a pop-up widget to display notifications. This is also used by the NotificationList

class CNotificationWidget : public QWidget
{
	Q_PROPERTY(i32 notificationDuration READ GetNotificationDuration WRITE SetNotificationDuration DESIGNABLE true)
	Q_OBJECT
public:
	CNotificationWidget(i32 notificationId, QWidget* pParent = nullptr);
	~CNotificationWidget();

	virtual QSize sizeHint() const override { return QSize(250, 90); }

	// The title will be appended to the current title if necessary and notification count will go up
	void           CombineNotifications(const QString& title);
	void           ShowAt(const QPoint& globalPos);
	void           Hide();
	void           StartHideTimer();

	bool           CanHide() const;
	bool           IsLogMessage() const { return m_bIsLogMessage; }
	i32            GetCount() const     { return m_count; }
	const QString& GetTitle() const     { return m_title; }
	QString        GetMessage() const;

	i32            GetNotificationDuration() const       { return notificationDuration; }
	void           SetNotificationDuration(i32 duration) { notificationDuration = duration; }
	void           OnAccepted();

protected:
	void         ProgressChanged(float value);
	void         MessageChanged(const QString& message);
	void         TaskComplete();

	void         OnHideTimerElapsed();

	virtual void mousePressEvent(QMouseEvent* pEvent) override;
	virtual void paintEvent(QPaintEvent* pEvent) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual void enterEvent(QEvent* pEvent) override;
	virtual void leaveEvent(QEvent* pEvent) override;

public:
	CDrxSignal<void()> signalNotificationHide;
	CDrxSignal<void()> signalNotificationHideAll;

protected:
	QString       m_title;
	QTimer*       m_pHideTimer;
	QLabel*       m_pTitle;
	QLabel*       m_pMessage;
	QLabel*       m_pIconLabel;
	QProgressBar* m_pProgressBar;
	i32           m_id; // id back to notification center data
	i32           m_count;
	i32           notificationDuration; // QSS Property
	bool          m_bIsLogMessage;
	bool          m_bIsAccepted; // User has seen and accepted this notification
	bool          m_bDisableTimerHide;
};

