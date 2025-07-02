// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"

#include "NotificationCenterTrayWidget.h"
#include "NotificationHistory.h"
#include "NotificationWidget.h"
#include "NotificationList.h"
#include "NotificationCenterImpl.h"

// EditorCommon
#include "EditorFramework/TrayArea.h"
#include "Controls/QPopupWidget.h"
#include "DrxIcon.h"
#include "EditorStyleHelper.h"
#include "QtUtil.h"

// Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QEvent>

REGISTER_TRAY_AREA_WIDGET(CNotificationCenterTrayWidget, 10)

class CNotificationPopupManager : public QObject
{
public:
	CNotificationPopupManager(CNotificationCenterTrayWidget* pNotificationCenterWidget)
		: QObject(pNotificationCenterWidget)
		, m_pNotificationCenterWidget(pNotificationCenterWidget)
		, notificationDistance(10)
		, animationDuration(250)
		, animationDistance(25)
		, notificationMaxCount(20)
		, m_bPreventPopups(true)
	{
	}

	~CNotificationPopupManager()
	{
		HideAllNotificationPopUps();
	}

public:
	void SpawnNotification(i32 id)
	{
		CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
		Internal::CNotification* pNotificationDesc = pNotificationCenter->GetNotification(id);

		if (!gNotificationPreferences.allowPopUps())
			return;

		if (pNotificationDesc->IsLogMessage())
		{
			for (CNotificationWidget* pNotification : m_notificationPopups)
			{
				if (pNotification && pNotification->IsLogMessage())
				{
					CombineDrxLogNotifications(pNotificationDesc);
					return;
				}
			}
		}

		CNotificationWidget* pNotification = new CNotificationWidget(pNotificationDesc->GetId());
		Add(pNotification);

		// Used for when the editor is loading up. Accumulate notifications to be displayed after
		// the main frame has finished loading. Otherwise notifications will spawn out of screen bounds
		if (m_bPreventPopups)
			return;

		Show(pNotification);
	}

	void ClearQueuedNotifications()
	{
		for (CNotificationWidget* pNotification : m_notificationPopups)
		{
			pNotification->deleteLater();
		}

		m_notificationPopups.clear();
	}

	void SetPreventPopUps(bool bPreventPopups)
	{
		m_bPreventPopups = bPreventPopups;

		// Try and spawn any queued up notification
		if (!m_bPreventPopups)
			ShowQueuedNotifications();
	}

	void HideAllNotificationPopUps()
	{
		for (auto i = 0; i < m_notificationPopups.size(); ++i)
		{
			if (!m_notificationPopups[i])
				continue;

			AnimateNotification(m_notificationPopups[i], false);
		}

		m_notificationPopups.clear();
		m_moveAnimations.clear();
	}

	i32  GetNotificationDistance() const       { return notificationDistance; }
	i32  GetNotificationMaxCount() const       { return notificationMaxCount; }
	i32  GetAnimationDuration() const          { return animationDuration; }
	i32  GetAnimationDistance() const          { return animationDistance; }

	void SetNotificationDistance(i32 distance) { notificationDistance = distance; }
	void SetNotificationMaxCount(i32 maxCount) { notificationMaxCount = maxCount; }
	void SetAnimationDuration(i32 duration)    { animationDuration = duration; }
	void SetAnimationDistance(i32 distance)    { animationDistance = distance; }

private:
	void AnimateNotification(CNotificationWidget* pNotification, bool bIn)
	{
		QPropertyAnimation* pAlphaAnim = new QPropertyAnimation(pNotification, "windowOpacity");
		pAlphaAnim->setDuration(animationDuration);
		pAlphaAnim->setStartValue(bIn ? 0.0f : 1.0f);
		pAlphaAnim->setEndValue(bIn ? 1.0 : 0.0f);

		AnimateMove(pNotification, QPoint(animationDistance, 0));

		pAlphaAnim->start(QAbstractAnimation::DeleteWhenStopped);

		if (!bIn)
		{
			connect(pAlphaAnim, &QPropertyAnimation::finished, [this, pNotification]()
			{
				pNotification->deleteLater();
			});
		}
	}

	void AnimateMove(CNotificationWidget* pNotification, const QPoint& delta)
	{
		// Check if there's a move animation already for this notification
		QPropertyAnimation* pMoveAnim = m_moveAnimations[pNotification];

		QRectF startValue = pNotification->geometry();
		QRectF endValue = startValue;

		if (!pMoveAnim)
		{
			endValue = QRectF(startValue.left() + delta.x(), startValue.top() + delta.y(), startValue.width(), startValue.height());
		}
		else
		{
			// If there was already an animation, make sure to take the existing end value and add the new delta
			startValue = pMoveAnim->currentValue().toRectF();
			endValue = pMoveAnim->endValue().toRectF();
			endValue = QRectF(endValue.left() + delta.x(), endValue.top() + delta.y(), endValue.width(), endValue.height());

			pMoveAnim->stop();
			m_moveAnimations.remove(pNotification);
		}

		pMoveAnim = new QPropertyAnimation(pNotification, "geometry");
		pMoveAnim->setDuration(animationDuration);
		pMoveAnim->setEasingCurve(QEasingCurve::OutCubic);
		pMoveAnim->setStartValue(startValue);
		pMoveAnim->setEndValue(endValue);
		pMoveAnim->start(QAbstractAnimation::DeleteWhenStopped);

		m_moveAnimations.insert(pNotification, pMoveAnim);

		connect(pMoveAnim, &QPropertyAnimation::finished, [this, pNotification, pMoveAnim]()
		{
			// Make sure we don't track this move anim after it's done playing
			if (m_moveAnimations.contains(pNotification) && pMoveAnim == m_moveAnimations[pNotification])
				m_moveAnimations.remove(pNotification);
		});
	}

	void MoveNotificationsUp(i32 idx, i32 delta)
	{
		for (; idx < m_notificationPopups.size(); ++idx)
		{
			CNotificationWidget* pNotification = m_notificationPopups[idx];
			AnimateMove(pNotification, QPoint(0, -delta));
		}
	}

	i32 Add(CNotificationWidget* pNotification)
	{
		m_notificationPopups.push_back(pNotification);
		return m_notificationPopups.size() - 1;
	}

	void Remove(CNotificationWidget* pNotification)
	{
		i32 idx = m_notificationPopups.indexOf(pNotification);
		if (idx != -1)
		{
			i32 freedHeight = pNotification->height();
			m_notificationPopups.removeAt(idx);
			// Move up pop-ups starting from idx
			MoveNotificationsUp(idx, freedHeight + notificationDistance);

			// Try and spawn any queued up notification
			ShowQueuedNotifications();
		}
	}

	bool Show(CNotificationWidget* pNotification)
	{
		i32 idx = m_notificationPopups.indexOf(pNotification);
		i32 yDelta = m_pNotificationCenterWidget->height() + notificationDistance;
		for (auto i = 0; i < idx; ++i)
		{
			yDelta += m_notificationPopups[i]->height() + notificationDistance;
		}

		pNotification->ShowAt(m_pNotificationCenterWidget->mapToGlobal(QPoint(0, yDelta)));

		// Get the screen geometry so we make sure we're not spawning a notification outside the screen
		// This must happen after the notification is shown or else it's dimensions will be incorrect
		QRect screenRes = QApplication::desktop()->screenGeometry();
		if ((yDelta + pNotification->height()) > screenRes.height())
		{
			pNotification->Hide();
			return false;
		}

		AnimateNotification(pNotification, true);
		pNotification->signalNotificationHide.Connect([this, pNotification] { Hide(pNotification); });
		pNotification->signalNotificationHideAll.Connect([this] { HideAll(); });
		return true;
	}

	void ShowQueuedNotifications()
	{
		for (i32 i = 0; i < m_notificationPopups.size(); ++i)
		{
			if (m_notificationPopups[i]->isVisible())
				continue;

			if (!Show(m_notificationPopups[i]))
				return;
		}
	}

	void Hide(CNotificationWidget* pNotification)
	{
		AnimateNotification(pNotification, false);
		// Another notification can now take it's place
		Remove(pNotification);
	}

	void HideAll()
	{
		for (i32 i = m_notificationPopups.size() - 1; i >= 0; --i)
		{
			if (m_notificationPopups[i]->isVisible())
			{
				AnimateNotification(m_notificationPopups[i], false);
				// Another notification can now take it's place
				Remove(m_notificationPopups[i]);
			}
			else
			{
				m_notificationPopups[i]->deleteLater();
				m_notificationPopups.removeAt(i);
			}
		}
	}

	void CombineDrxLogNotifications(Internal::CNotification* pNotificationDesc)
	{
		if (!pNotificationDesc->IsLogMessage())
			return;

		for (auto i = 0; i < m_notificationPopups.size(); ++i)
		{
			if (m_notificationPopups[i] && m_notificationPopups[i]->IsLogMessage())
			{
				CNotificationWidget* pNotification = m_notificationPopups[i];
				// Capture height before combining
				QSize prevSize = pNotification->size();
				// Combine titles, increase message count display and start timer to hide the notification
				pNotification->CombineNotifications(pNotificationDesc->GetTitle());
				pNotification->StartHideTimer();
				// Capture height after combining and calculate delta to check if some space freed up
				QSize delta = prevSize - pNotification->size();

				// If there's already an existing move animation make sure to update the size in the animation
				// If not the widget's geometry will keep animating with an outdated size
				QPropertyAnimation* pMoveAnim = m_moveAnimations[pNotification];
				if (pMoveAnim && !delta.isNull())
				{
					QRectF startValue = pNotification->geometry();
					startValue.setSize(pNotification->size());
					QRectF endValue = pMoveAnim->endValue().toRectF();
					endValue.setSize(pNotification->size());
					i32 timeRemaining = pMoveAnim->duration() - pMoveAnim->currentTime();
					pMoveAnim->stop();
					m_moveAnimations.remove(pNotification);

					pMoveAnim = new QPropertyAnimation(pNotification, "geometry");
					pMoveAnim->setDuration(timeRemaining);
					pMoveAnim->setEasingCurve(QEasingCurve::OutCubic);
					pMoveAnim->setStartValue(startValue);
					pMoveAnim->setEndValue(endValue);
					pMoveAnim->start(QAbstractAnimation::DeleteWhenStopped);

					m_moveAnimations.insert(pNotification, pMoveAnim);

					connect(pMoveAnim, &QPropertyAnimation::finished, [this, pNotification, pMoveAnim]()
					{
						// Make sure we don't track this move anim after it's done playing
						if (m_moveAnimations.contains(pNotification) && pMoveAnim == m_moveAnimations[pNotification])
							m_moveAnimations.remove(pNotification);
					});
				}

				if (delta.height() > 0) // If there's a positive delta, then move all other notifications up
					MoveNotificationsUp(i + 1, delta.height());
				break;
			}
		}
	}

private:
	// QSS properties
	i32 notificationDistance;
	i32 animationDuration;
	i32 animationDistance;
	i32 notificationMaxCount;

	//
	i32                                              m_lastShownIdx;
	QWidget*                                         m_pNotificationCenterWidget;
	QVector<CNotificationWidget*>                    m_notificationPopups;
	// Ongoing geometry animations need to be cached so we can update the end value if needed
	QHash<CNotificationWidget*, QPropertyAnimation*> m_moveAnimations;

	// Used for when the editor is loading up. Accumulate notifications to be displayed after
	// the main frame has finished loading. Otherwise notifications will spawn out of screen bounds
	bool m_bPreventPopups;
};

class CNotificationCenterWidget : public QWidget
{
public:
	CNotificationCenterWidget(QWidget* pParent = nullptr)
		: QWidget(pParent)
	{
		QVBoxLayout* pMainLayout = new QVBoxLayout();
		pMainLayout->setSpacing(0);
		pMainLayout->setMargin(0);

		QTabWidget* pTabWidget = new QTabWidget();
		pTabWidget->setObjectName("notificationTabs");
		pTabWidget->addTab(new CNotificationList(), "Current");
		pTabWidget->addTab(new CNotificationHistory(), "History");
		pTabWidget->addTab(GetIEditor()->GetPreferences()->GetPageWidget("Notifications/General"), "Preferences");
		pMainLayout->addWidget(pTabWidget);
		pMainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
		setLayout(pMainLayout);
	}
};

CNotificationCenterDockable::CNotificationCenterDockable(QWidget* pParent /* = nullptr*/)
	: CDockableWidget(pParent)
{
	QVBoxLayout* pMainLayout = new QVBoxLayout();
	pMainLayout->setSpacing(0);
	pMainLayout->setMargin(0);
	pMainLayout->addWidget(new CNotificationCenterWidget());
	setLayout(pMainLayout);
}

CNotificationCenterTrayWidget::CNotificationCenterTrayWidget(QWidget* pParent /* = nullptr*/)
	: QToolButton(pParent)
	, m_pPopUpMenu(nullptr)
	, m_notificationType(-1)
{
	m_pPopUpManager = new CNotificationPopupManager(this);
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());

	connect(pNotificationCenter, &CNotificationCenter::NotificationAdded, this, &CNotificationCenterTrayWidget::OnNotificationAdded);
	connect(pNotificationCenter, &CNotificationCenter::NotificationAdded, m_pPopUpManager, &CNotificationPopupManager::SpawnNotification);
	connect(this, &QToolButton::clicked, this, &CNotificationCenterTrayWidget::OnClicked);
	connect(GetIEditor()->GetTrayArea(), &CTrayArea::MainFrameInitialized, [this]()
	{
		m_pPopUpMenu = new QPopupWidget("NotificationCenterPopup", QtUtil::MakeScrollable(new CNotificationCenterWidget()), QSize(480, 640));
		m_pPopUpMenu->SetFocusShareWidget(this);
		m_pPopUpMenu->installEventFilter(this); // used to catch show and hide events

		m_pPopUpManager->SetPreventPopUps(false);
	});

	setIcon(DrxIcon("icons:Dialogs/notification_text.ico"));
}

CNotificationCenterTrayWidget::~CNotificationCenterTrayWidget()
{
	delete m_pPopUpMenu;
}

bool CNotificationCenterTrayWidget::eventFilter(QObject* pWatched, QEvent* pEvent)
{
	if (pWatched != m_pPopUpMenu)
		return false;

	if (pEvent->type() == QEvent::Hide)
	{
		// Clear queued up notifications and enable pop ups to be created
		m_pPopUpManager->ClearQueuedNotifications();
		m_pPopUpManager->SetPreventPopUps(false);
	}
	else if (pEvent->type() == QEvent::Show)
	{
		// Prevent popups while the dialog is open
		m_pPopUpManager->SetPreventPopUps(true);
	}

	return false;
}

void CNotificationCenterTrayWidget::OnNotificationAdded(i32 id)
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	Internal::CNotification* pNotificationDesc = pNotificationCenter->GetNotification(id);

	if (m_pPopUpMenu && !m_pPopUpMenu->isVisible())
	{
		Internal::CNotification::Type notificationType = pNotificationDesc->GetType();
		if (m_notificationType >= notificationType)
			return;

		if (notificationType != Internal::CNotification::Progress)
			m_notificationType = notificationType;

		DrxIconColorMap colorMap = DrxIconColorMap();

		switch (m_notificationType)
		{
		case Internal::CNotification::Type::Warning:
			colorMap[QIcon::Selected] = GetStyleHelper()->warningColor();
			setIcon(DrxIcon("icons:Dialogs/notification_warning.ico", colorMap).pixmap(24, 24, QIcon::Normal, QIcon::On));
			break;
		case Internal::CNotification::Type::Critical:
			colorMap[QIcon::Selected] = GetStyleHelper()->errorColor();
			setIcon(DrxIcon("icons:Dialogs/notification_warning.ico", colorMap).pixmap(24, 24, QIcon::Normal, QIcon::On));
			break;
		default:
			colorMap[QIcon::Selected] = GetStyleHelper()->activeIconTint();
			setIcon(DrxIcon("icons:Dialogs/notification_text.ico", colorMap).pixmap(24, 24, QIcon::Normal, QIcon::On));
			break;
		}
	}
}

void CNotificationCenterTrayWidget::OnClicked(bool bChecked)
{
	if (!m_pPopUpMenu->isVisible())
	{
		m_notificationType = -1;
		setIcon(DrxIcon("icons:Dialogs/notification_text.ico"));
	}

	if (m_pPopUpMenu->isVisible())
	{
		m_pPopUpMenu->hide();
		return;
	}

	m_pPopUpMenu->ShowAt(mapToGlobal(QPoint(width(), height())));
	m_pPopUpManager->HideAllNotificationPopUps();
}

i32 CNotificationCenterTrayWidget::GetNotificationDistance() const
{
	return m_pPopUpManager->GetNotificationDistance();
}

i32 CNotificationCenterTrayWidget::GetNotificationMaxCount() const
{
	return m_pPopUpManager->GetNotificationMaxCount();
}

i32 CNotificationCenterTrayWidget::GetAnimationDuration() const
{
	return m_pPopUpManager->GetAnimationDuration();
}

i32 CNotificationCenterTrayWidget::GetAnimationDistance() const
{
	return m_pPopUpManager->GetAnimationDistance();
}

void CNotificationCenterTrayWidget::SetNotificationDistance(i32 distance)
{
	m_pPopUpManager->SetNotificationDistance(distance);
}

void CNotificationCenterTrayWidget::SetNotificationMaxCount(i32 maxCount)
{
	m_pPopUpManager->SetNotificationMaxCount(maxCount);
}

void CNotificationCenterTrayWidget::SetAnimationDuration(i32 duration)
{
	m_pPopUpManager->SetAnimationDuration(duration);
}

void CNotificationCenterTrayWidget::SetAnimationDistance(i32 distance)
{
	m_pPopUpManager->SetAnimationDistance(distance);

}

