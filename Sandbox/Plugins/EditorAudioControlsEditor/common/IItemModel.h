// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>

#include "FileImportInfo.h"

#include <QHeaderView>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <FileDialogs/ExtensionFilter.h>

namespace ACE
{
namespace Impl
{
using ColumnResizeModes = std::map<i32, QHeaderView::ResizeMode>;

struct IItemModel : public QAbstractItemModel
{
	//! \cond INTERNAL
	virtual ~IItemModel() = default;
	//! \endcond

	//! Returns the logical index of the name column. Used for filtering.
	virtual i32 GetNameColumn() const = 0;

	//! Returns the logical indexes of columns and their resize modes.
	//! Columns that are not returned will be set to interactive resize mode.
	virtual ColumnResizeModes const& GetColumnResizeModes() const = 0;

	//! Returns the name of the current selected folder item, if supported by the implementation.
	//! Used for file import.
	//! \param index - Index of the current selected item.
	virtual QString const GetTargetFolderName(QModelIndex const& index) const = 0;

	//! Gets the supported file types.
	//! Used for file import.
	virtual QStringList const& GetSupportedFileTypes() const = 0;

	//! Gets the extensions and their descriptions for file types that are allowed to get imported.
	//! Used for file import.
	virtual ExtensionFilterVector const& GetExtensionFilters() const = 0;

	//! Sends a signal when files get dropped, if the middleware allows file import.
	//! \param SFileImportInfo - List of file infos of the dropped files.
	//! \param QString - Name of the target folder on which the files are dropped. If files are dropped on the root, this string has to be empty.
	CDrxSignal<void(FileImportInfos const&, QString const&)> SignalDroppedFiles;
};
} //endns Impl
} //endns ACE

