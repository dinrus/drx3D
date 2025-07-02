// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QString>

namespace FileSystem
{
namespace Internal
{
namespace Win32
{

/// \return absolute path where the link points to (empty if not a link)
/// \note assure that linkPath is a path
QString GetLinkTargetPath(const QString& linkPath);

} //endns Win32
} //endns Internal
} //endns FileSystem

