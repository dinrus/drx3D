// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"

#include "NotificationModel_Internal.h"
#include "Notifications/NotificationCenterImpl.h"

// Qt
#include <QClipboard>

// EditorCommon
#include "DrxIcon.h"

// Editor
#include "IEditor.h"

tukk CNotificationModel::s_ColumnNames[s_ColumnCount] = { QT_TR_NOOP("Type"), QT_TR_NOOP("Time"), QT_TR_NOOP("Title"), QT_TR_NOOP("Message") };

CNotificationFilterModel::CNotificationFilterModel(QObject* pParent /* = nullptr*/)
	: QSortFilterProxyModel(pParent)
	, m_typeMask(-1)
{
}

void CNotificationFilterModel::SetFilterTypeState(unsigned type, bool bEnable)
{
	unsigned mask = 1 << type;
	m_typeMask = (m_typeMask & ~mask) | bEnable * mask;
	invalidateFilter();
}

bool CNotificationFilterModel::filterAcceptsRow(i32 sourceRow, const QModelIndex& sourceParent) const
{
	unsigned notificationType = sourceModel()->index(sourceRow, 0, sourceParent).data(static_cast<i32>(CNotificationModel::Roles::Type)).toUInt();
	return m_typeMask & (1 << notificationType);
}

//////////////////////////////////////////////////////////////////////////

CNotificationModel::CNotificationModel(QObject* pParent /* = nullptr*/)
	: QAbstractItemModel(pParent)
	, m_startIdx(0)
	, m_startSize(0)
	, m_currIdx(static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter())->GetNotificationCount() - 1)
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	connect(pNotificationCenter, &CNotificationCenter::NotificationAdded, this, &CNotificationModel::OnNotificationAdded);
}

void CNotificationModel::OnNotificationAdded(i32 id)
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	i32 idx = pNotificationCenter->GetNotificationCount() - m_startIdx - 1;
	if (idx > m_currIdx)
	{
		beginInsertRows(QModelIndex(), m_currIdx + 1, idx);
		m_currIdx = idx;
		endInsertRows();
	}
}

void CNotificationModel::CopyHistory() const
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());

	auto notificationCount = pNotificationCenter->GetNotificationCount();
	QString notificationHistory;
	for (auto i = m_startIdx; i < notificationCount; ++i)
	{
		Internal::CNotification* pNotification = pNotificationCenter->GetNotification(i);
		if (pNotification->GetType() == Internal::CNotification::Progress)
			continue;

		notificationHistory += pNotification->GetDetailedMessage() + "\n";
	}

	QApplication::clipboard()->setText(notificationHistory);
}

void CNotificationModel::CopyHistory(QModelIndexList indices) const
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());

	QString notificationHistory;
	for (auto index : indices)
	{
		i32k notificationIdx = m_startIdx + index.row();
		Internal::CNotification* pNotification = pNotificationCenter->GetNotification(notificationIdx);
		if (pNotification->GetType() == Internal::CNotification::Progress)
			continue;

		notificationHistory += pNotification->GetDetailedMessage() + "\n";
	}

	QApplication::clipboard()->setText(notificationHistory);
}

void CNotificationModel::ClearAll()
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	beginResetModel();
	m_startIdx = pNotificationCenter->GetNotificationCount();
	m_startSize = m_startIdx;
	m_currIdx = -1;
	endResetModel();
}

void CNotificationModel::ShowCompleteHistory()
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	beginResetModel();
	m_startIdx = 0;
	m_startSize = 0;
	m_currIdx = pNotificationCenter->GetNotificationCount() - 1;
	endResetModel();
}

QVariant CNotificationModel::data(const QModelIndex& index, i32 role /* = Qt::DisplayRole */) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());

	// Determine the real index for this notification. It should be the start index + the actual row
	i32k notificationIdx = m_startIdx + index.row();
	Internal::CNotification* pNotification = pNotificationCenter->GetNotification(notificationIdx);

	switch (role)
	{
	case Qt::DecorationRole:
		{
			if (index.column() == 0)
			{
				switch (pNotification->GetType())
				{
				case Internal::CNotification::Info:
					return DrxIcon("icons:Dialogs/dialog-question.ico");
					break;
				case Internal::CNotification::Warning:
					return DrxIcon("icons:Dialogs/dialog-warning.ico");
					break;
				case Internal::CNotification::Critical:
					return DrxIcon("icons:Dialogs/dialog-error.ico");
					break;
				case Internal::CNotification::Progress:
					return DrxIcon("icons:Dialogs/dialog_loading.ico");
					break;
				}
			}
			else if (index.column() == 3)
			{
				if (pNotification->GetCommandCount())
					return DrxIcon("icons:Dialogs/notification-has-command.ico");
			}

			return QVariant();
		}
	case Qt::DisplayRole:
	case Qt::EditRole:
		{
			switch (index.column())
			{
			case 0:
				return QVariant();
			case 1:
				{
					const QDateTime& dateTime = pNotification->GetDateTime();
					if (dateTime.date() != QDateTime::currentDateTime().date())
						return dateTime;

					return dateTime.time();
				}
			case 2:
				return pNotification->GetTitle();
			case 3:
				return pNotification->GetMessage();
			}
		}
		break;
	case Roles::SortRole:
	{
		switch (index.column())
		{
		case 0:
			return pNotification->GetType();
		case 1:
		{
			const QDateTime& dateTime = pNotification->GetDateTime();
			if (dateTime.date() != QDateTime::currentDateTime().date())
				return dateTime;

			return dateTime.time();
		}
		case 2:
			return pNotification->GetTitle();
		case 3:
			return pNotification->GetMessage();
		}
	}
	case Roles::Type:
		return pNotification->GetType();
		break;
	}

	return QVariant();
}

bool CNotificationModel::hasChildren(const QModelIndex& parent /* = QModelIndex() */) const
{
	if (!parent.isValid())
	{
		CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
		return (pNotificationCenter->GetNotificationCount() - m_startSize) != 0;
	}
	return false;
}

QVariant CNotificationModel::headerData(i32 section, Qt::Orientation orientation, i32 role /* = Qt::DisplayRole */) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		return tr(s_ColumnNames[section]);
	}
	else
	{
		return QVariant();
	}
}

Qt::ItemFlags CNotificationModel::flags(const QModelIndex& index) const
{
	return QAbstractItemModel::flags(index);
}
QModelIndex CNotificationModel::index(i32 row, i32 column, const QModelIndex& parent /* = QModelIndex()*/) const
{
	return createIndex(row, column);
}

QModelIndex CNotificationModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

bool CNotificationModel::setData(const QModelIndex& index, const QVariant& value, i32 role /*= Qt::EditRole*/)
{
	return false;
}

i32 CNotificationModel::rowCount(const QModelIndex& parent /* = QModelIndex() */) const
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());
	return pNotificationCenter->GetNotificationCount() - m_startSize;
}

Internal::CNotification* CNotificationModel::NotificationFromIndex(const QModelIndex& index) const
{
	CNotificationCenter* pNotificationCenter = static_cast<CNotificationCenter*>(GetIEditor()->GetNotificationCenter());

	// Determine the real index for this notification. It should be the start index + the actual row
	i32k notificationIdx = m_startIdx + index.row();

	return pNotificationCenter->GetNotification(notificationIdx);
}

