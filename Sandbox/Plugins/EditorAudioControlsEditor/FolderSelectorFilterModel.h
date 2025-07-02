// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <ProxyModels/DeepFilterProxyModel.h>

namespace ACE
{
class CFolderSelectorFilterModel final : public QDeepFilterProxyModel
{
public:

	explicit CFolderSelectorFilterModel(QString const& assetpath, QObject* const pParent);

	CFolderSelectorFilterModel() = delete;

protected:

	// QDeepFilterProxyModel
	virtual bool rowMatchesFilter(i32 sourceRow, QModelIndex const& sourcePparent) const override;
	// ~QDeepFilterProxyModel

private:

	QString const m_assetPath;
};
} //endns ACE
