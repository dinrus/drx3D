// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

namespace FileSystem
{
namespace Internal
{
namespace Win32
{

/// \brief internal win32 specific sequence of action to pathes
struct SActionSequence
{
	enum ActionType { Created, Removed, Modified, RenamedFrom, RenamedTo };

	struct SAction
	{
		ActionType type;
		QString    fullPath;
	};

	u64    key;                  ///< unique key associated with the base path
	QString          baseFullAbsolutePath; ///< monitored base path (all pathes are relative to this)
	QVector<SAction> sequence;
};

} //endns Win32
} //endns Internal
} //endns FileSystem

// allow to use with signal & slots
Q_DECLARE_METATYPE(FileSystem::Internal::Win32::SActionSequence)

