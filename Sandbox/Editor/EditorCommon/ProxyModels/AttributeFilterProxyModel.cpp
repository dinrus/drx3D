// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AttributeFilterProxyModel.h"
#include "ProxyModels/ItemModelAttribute.h"
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

QAttributeFilterProxyModel::QAttributeFilterProxyModel(BehaviorFlags behavior /*= AcceptIfChildMatches*/, QObject* pParent /*= nullptr*/, i32 role /*= Attributes::s_defaultRole*/)
	: m_role(role)
	, QDeepFilterProxyModel(behavior, pParent)
{
}

QAttributeFilterProxyModel::~QAttributeFilterProxyModel()
{
	ClearFilters();
}

void QAttributeFilterProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
	auto oldsource = sourceModel();
	if (oldsource)
	{
		disconnect(oldsource, &QAbstractItemModel::columnsRemoved, this, &QAttributeFilterProxyModel::OnColumnsRemoved);
		disconnect(oldsource, &QAbstractItemModel::columnsInserted, this, &QAttributeFilterProxyModel::OnColumnsInserted);
		disconnect(oldsource, &QAbstractItemModel::columnsMoved, this, &QAttributeFilterProxyModel::OnColumnsMoved);
		disconnect(oldsource, &QAbstractItemModel::modelReset, this, &QAttributeFilterProxyModel::ResetAttributes);
	}

	QDeepFilterProxyModel::setSourceModel(pSourceModel);

	if (pSourceModel != oldsource)
	{
		ResetAttributes();

		if (pSourceModel)
		{
			connect(pSourceModel, &QAbstractItemModel::columnsRemoved, this, &QAttributeFilterProxyModel::OnColumnsRemoved);
			connect(pSourceModel, &QAbstractItemModel::columnsInserted, this, &QAttributeFilterProxyModel::OnColumnsInserted);
			connect(pSourceModel, &QAbstractItemModel::columnsMoved, this, &QAttributeFilterProxyModel::OnColumnsMoved);
			connect(pSourceModel, &QAbstractItemModel::modelReset, this, &QAttributeFilterProxyModel::ResetAttributes);
		}
	}
}

void QAttributeFilterProxyModel::SetAttributeRole(i32 role)
{
	m_role = role;
	ResetAttributes();
	invalidate();
}

void QAttributeFilterProxyModel::AddFilter(AttributeFilterSharedPtr pFilter)
{
	if (pFilter)
	{
		m_filters.push_back(pFilter);
		InvalidateFilter();
	}
}

void QAttributeFilterProxyModel::RemoveFilter(AttributeFilterSharedPtr pFilter)
{
	if (pFilter)
	{
		stl::find_and_erase(m_filters, pFilter);
		InvalidateFilter();
	}
}

void QAttributeFilterProxyModel::ClearFilters()
{
	m_filters.clear();
	InvalidateFilter();
}

bool QAttributeFilterProxyModel::rowMatchesFilter(i32 sourceRow, const QModelIndex& sourceParent) const
{
	if (QDeepFilterProxyModel::rowMatchesFilter(sourceRow, sourceParent))
	{
		for (auto filter : m_filters)
		{
			if (filter->IsEnabled())
			{
				//TODO : investigate speedup of using sibling() instead of index() for every filter.
				//This should allow expensive models to optimize their behavior. Also applies to other models
				QModelIndex index = sourceModel()->index(sourceRow, m_attributes.at(filter->GetAttribute()), sourceParent);
				if (index.isValid())
				{
					i32k role = filter->GetAttribute()->GetType() == eAttributeType_Boolean ? Qt::CheckStateRole : Qt::DisplayRole;
					QVariant val = sourceModel()->data(index, role);
					if (!filter->Match(val))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	return false;
}

void QAttributeFilterProxyModel::ResetAttributes()
{
	m_attributes.clear();
	QAbstractItemModel* pModel = sourceModel();
	if (pModel)
	{
		i32k size = pModel->columnCount();
		for (i32 i = 0; i < size; ++i)
		{
			QVariant value = pModel->headerData(i, Qt::Horizontal, m_role);
			CItemModelAttribute* pAttribute = value.value<CItemModelAttribute*>();
			if (pAttribute)
			{
				m_attributes[pAttribute] = i;
			}
		}
	}
	signalAttributesChanged();
}

void QAttributeFilterProxyModel::OnColumnsRemoved(const QModelIndex& parent, i32 first, i32 last)
{
	ResetAttributes();
}

void QAttributeFilterProxyModel::OnColumnsInserted(const QModelIndex& parent, i32 first, i32 last)
{
	ResetAttributes();
}

void QAttributeFilterProxyModel::OnColumnsMoved(const QModelIndex& parent, i32 start, i32 end, const QModelIndex& destination, i32 column)
{
	ResetAttributes();
}

