// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "FileSystem_Internal_Win32_UniqueFind.h"

#include "FileSystem_Internal_Win32_Utils.h"

#include <QDir>

namespace FileSystem
{
namespace Internal
{
namespace Win32
{

QString CUniqueFind::GetLinkTargetPath() const
{
	if (!IsLink())
	{
		return QString();
	}
	return FileSystem::Internal::Win32::GetLinkTargetPath(QDir(m_path).filePath(GetFileName()));
}

bool CUniqueFind::FindFirst(const QString& path)
{
	Close();
	auto searchMask = QStringLiteral("\\\\?\\") + QDir::toNativeSeparators(QDir(path).filePath("*"));
	auto searchMaskWStr = searchMask.toStdWString();
	m_path = path;
	m_handle = FindFirstFileExW(
	  searchMaskWStr.data(),       // FileName
	  FindExInfoBasic, &m_data,    // InfoLevel, FindData
	  FindExSearchNameMatch, NULL, // SearchOp, searchFilter
	  FIND_FIRST_EX_LARGE_FETCH    // AdditionalFlags
	  );
	return IsValid();
}

} //endns Win32
} //endns Internal
} //endns FileSystem

