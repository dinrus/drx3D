// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Qt>

#include <QAbstractItemModel>

enum EItemDataRole
{
	// Returns variant of type uk which points to a Serialization::SStruct.
	// The caller of data() is responsible for deletion.
	eItemDataRole_YasliSStruct = Qt::UserRole,

	eItemDataRole_MAX
};

QModelIndex GetSourceModelIndex(const QModelIndex& index);

