// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QFileSystemModel>
#include <QHash>

/**
 * \brief Replacement for QFileSystemModel. Do not use QFileSystemModel
 * * fixes the alignment of the second column
 * * overrides default icons for folders and files using our own icons
 * * fixes crashes due to icon requests - check https://bugreports.qt.io/browse/QTBUG-48823
 */
class EDITOR_COMMON_API CAdvancedFileSystemModel
	: public QFileSystemModel
{
	// as we do not need registered QObject behavior it's not registered as such
public:
	explicit CAdvancedFileSystemModel(QObject* parent = nullptr);

	// interface QAbstractItemModel
	virtual QVariant data(const QModelIndex& index, i32 role = Qt::DisplayRole) const override;
};

