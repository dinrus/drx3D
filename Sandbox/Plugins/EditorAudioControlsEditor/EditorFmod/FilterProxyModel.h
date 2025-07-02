// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <ProxyModels/AttributeFilterProxyModel.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
class CFilterProxyModel final : public QAttributeFilterProxyModel
{
public:

	explicit CFilterProxyModel(QObject* const pParent);

	CFilterProxyModel() = delete;

private:

	// QAttributeFilterProxyModel
	virtual bool rowMatchesFilter(i32 sourceRow, QModelIndex const& sourcePparent) const override;
	// ~QAttributeFilterProxyModel

	// QSortFilterProxyModel
	virtual bool lessThan(QModelIndex const& left, QModelIndex const& right) const override;
	// ~QSortFilterProxyModel
};
} //endns Fmod
} //endns Impl
} //endns ACE
