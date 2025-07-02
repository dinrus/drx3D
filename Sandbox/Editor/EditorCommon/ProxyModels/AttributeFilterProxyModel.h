// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QSortFilterProxyModel>

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include "ProxyModels/DeepFilterProxyModel.h"
#include "ProxyModels/ItemModelAttribute.h"

typedef std::map<CItemModelAttribute*, i32> AttributeIndexMap;

//! Proxy model class to filter based on attributes. Used with QFilteringPanel
// Assumes the source model's columns are stable
class EDITOR_COMMON_API QAttributeFilterProxyModel : public QDeepFilterProxyModel
{
public:
	QAttributeFilterProxyModel(BehaviorFlags behavior = AcceptIfChildMatches, QObject* pParent = nullptr, i32 role = Attributes::s_getAttributeRole);
	virtual ~QAttributeFilterProxyModel();
	virtual void setSourceModel(QAbstractItemModel* pSourceModel) override;
	void         SetAttributeRole(i32 role);
	void         AddFilter(AttributeFilterSharedPtr pFilter);
	void         RemoveFilter(AttributeFilterSharedPtr pFilter);
	void         ClearFilters();

	void         InvalidateFilter() { QDeepFilterProxyModel::invalidateFilter(); }

	CDrxSignal<void()> signalAttributesChanged;

protected:
	virtual bool rowMatchesFilter(i32 sourceRow, const QModelIndex& sourceParent) const override;

private:
	void ResetAttributes();

	void OnColumnsRemoved(const QModelIndex& parent, i32 first, i32 last);
	void OnColumnsInserted(const QModelIndex& parent, i32 first, i32 last);
	void OnColumnsMoved(const QModelIndex& parent, i32 start, i32 end, const QModelIndex& destination, i32 column);

	friend class QFilteringPanel;

	i32                                   m_role;
	AttributeIndexMap                     m_attributes;

protected:
	std::vector<AttributeFilterSharedPtr> m_filters;
};

