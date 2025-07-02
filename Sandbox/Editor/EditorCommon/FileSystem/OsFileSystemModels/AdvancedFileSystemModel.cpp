// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "AdvancedFileSystemModel.h"

#include <QFileIconProvider>

#include "DrxIcon.h"
#include "FileSystem/FileType.h"

namespace  Private_AFSM_cpp
{
// Custom icon provider for QFileSystemModel also fixes bug with qt 5.6 - see https://bugreports.qt.io/browse/QTBUG-48823
class IconProvider : public QFileIconProvider
{
public:
	IconProvider()
		: QFileIconProvider()
	{
		setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
	}
	~IconProvider()
	{
	}

	virtual QIcon icon(IconType type) const override
	{
		switch (type)
		{
		case IconType::Folder:
			return DrxIcon(SFileType::DirectoryType()->iconPath).pixmap(16, 16);
			break;
		case IconType::File:
			return DrxIcon(SFileType::Unknown()->iconPath).pixmap(16, 16);
			break;
		}

		return QFileIconProvider::icon(type);
	}
	// Return the pixmap correspoding to DrxIcon's normal state so we can have tinted icons.
	// This way when the icon is active it'll still have the default normal state pixmap rather than the active pixmap (currently blue)
	virtual QIcon icon(const QFileInfo& info) const override
	{
		if (info.isSymLink())
		{
			return DrxIcon(SFileType::SymLink()->iconPath).pixmap(16, 16);
		}
		else if (info.isDir())
		{
			return DrxIcon(SFileType::DirectoryType()->iconPath).pixmap(16, 16);
		}
		else
		{
			return DrxIcon(SFileType::Unknown()->iconPath).pixmap(16, 16);
		}
	}
};
}
CAdvancedFileSystemModel::CAdvancedFileSystemModel(QObject* parent)
	: QFileSystemModel(parent)
{
	setIconProvider(new Private_AFSM_cpp::IconProvider);
}

QVariant CAdvancedFileSystemModel::data(const QModelIndex& index, i32 role) const
{
	if ((role == Qt::TextAlignmentRole) && (index.column() == 1))
	{
		return (i32)(Qt::AlignRight | Qt::AlignVCenter);
	}
	return QFileSystemModel::data(index, role);
}

