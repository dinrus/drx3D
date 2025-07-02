// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <ProxyModels/AttributeFilterProxyModel.h>

namespace ACE
{
class CSystemFilterProxyModel final : public QAttributeFilterProxyModel
{
public:

	CSystemFilterProxyModel(QObject* const pParent);

protected:

	// QAttributeFilterProxyModel
	virtual bool rowMatchesFilter(i32 sourceRow, QModelIndex const& sourcePparent) const override;
	// ~QAttributeFilterProxyModel

	// QSortFilterProxyModel
	virtual bool lessThan(QModelIndex const& left, QModelIndex const& right) const override;
	// ~QSortFilterProxyModel
};
} //endns ACE
