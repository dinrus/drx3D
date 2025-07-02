// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "FavoritesHelper.h"

#include "IEditor.h"
#include "EditorFramework/PersonalizationManager.h"
#include "QAdvancedItemDelegate.h"

#include <QTreeView>
#include <QHeaderView>

FavoritesHelper::FavoritesHelper(const QString& uniqueFavKey, i32 favoritesColumn)
	: m_key(uniqueFavKey)
	, m_favIdRole(s_FavoritesIdRole)
	, m_favColumn(favoritesColumn)
{
	Load();
}

FavoritesHelper::~FavoritesHelper()
{
}

void FavoritesHelper::SetupView(QAbstractItemView* view, QAdvancedItemDelegate* delegate, i32 favoritesColumn)
{
	bool multiselect = view->selectionMode() != QAbstractItemView::SingleSelection;
	if (multiselect)
		delegate->SetColumnBehavior(favoritesColumn, QAdvancedItemDelegate::BehaviorFlags(QAdvancedItemDelegate::OverrideCheckIcon | QAdvancedItemDelegate::MultiSelectionEdit));
	else
		delegate->SetColumnBehavior(favoritesColumn, QAdvancedItemDelegate::BehaviorFlags(QAdvancedItemDelegate::OverrideCheckIcon));
}

void FavoritesHelper::SetupView(QTreeView* treeView, QAdvancedItemDelegate* delegate, i32 favoritesColumn)
{
	treeView->header()->setMinimumSectionSize(16);
	treeView->header()->setSectionResizeMode(favoritesColumn, QHeaderView::Fixed);
	treeView->header()->resizeSection(favoritesColumn, 16);

	SetupView((QAbstractItemView*)treeView, delegate, favoritesColumn);
}

bool FavoritesHelper::IsFavorite(const QModelIndex& index) const
{
	return m_favorites.contains(index.data(m_favIdRole));
}

void FavoritesHelper::SetFavorite(const QModelIndex& index, bool favorite /*= true*/)
{
	QVariant id = index.data(m_favIdRole);
	if (favorite && !m_favorites.contains(id))
	{
		m_favorites.append(id);
		Save();
	}
	else if(!favorite && m_favorites.contains(id))
	{
		m_favorites.removeOne(id);
		Save();
	}
}

QVariant FavoritesHelper::data(const QModelIndex& index, i32 role) const
{
	if (index.column() == m_favColumn)
	{
		switch (role)
		{
		case Qt::CheckStateRole:
			return IsFavorite(index) ? Qt::Checked : Qt::Unchecked;
		case QAdvancedItemDelegate::s_IconOverrideRole:
		{
			const bool bIsFavorite = IsFavorite(index);
			return GetFavoriteIcon(bIsFavorite);
		}
		default:
			break;
		}
	}

	return QVariant();
}

bool FavoritesHelper::setData(const QModelIndex& index, const QVariant& value, i32 role)
{
	if (index.column() == m_favColumn && role == Qt::CheckStateRole)
	{
		SetFavorite(index, value.toBool());
		const_cast<QAbstractItemModel*>(index.model())->dataChanged(index, index, QVector<i32>() << Qt::CheckStateRole);
		return true;
	}
	return false;
}

Qt::ItemFlags FavoritesHelper::flags(const QModelIndex& index) const
{
	if (index.column() == m_favColumn)
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
	}
	else
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled; // defaults from QAbstractItemModel
	}
}

DrxIcon FavoritesHelper::GetFavoriteIcon(bool isActive)
{
	if (isActive)
		return DrxIcon("icons:General/Favorites_Full.ico");
	else
		return DrxIcon("icons:General/Favorites_Empty.ico");
}

void FavoritesHelper::Load()
{
	auto pm = GetIEditor()->GetPersonalizationManager();
	if (pm)
	{
		m_favorites.clear();
		m_favorites = pm->GetProjectProperty("FavoritesHelper", m_key).toList();
	}
}

void FavoritesHelper::Save()
{
	auto pm = GetIEditor()->GetPersonalizationManager();
	if (pm)
	{
		pm->SetProjectProperty("FavoritesHelper", m_key, m_favorites);
	}
}

