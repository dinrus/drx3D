// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "QSortFilterProxyModel"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class EDITOR_COMMON_API CFileSortProxyModel
	: public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit CFileSortProxyModel(QObject* parent = 0);

	// QSortFilterProxyModel interface
protected:
	virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
};

