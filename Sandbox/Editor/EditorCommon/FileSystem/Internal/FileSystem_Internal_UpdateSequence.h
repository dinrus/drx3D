// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_Internal_Data.h"

#include <QMetaType>
#include <QVector>

namespace FileSystem
{
namespace Internal
{

/// \brief internal sequence of updates from a monitor
class CUpdateSequence
{
public:
	template<
	  typename RemovePathCallback, typename RenamePathCallback,
	  typename CreateDirectoryCallback, typename CreateFileCallback,
	  typename ModifyDirectoryCallback, typename ModifyFileCallback>
	void visitAll(
	  const RemovePathCallback& removePathCallback, const RenamePathCallback& renamePathCallback,
	  const CreateDirectoryCallback& createDirectoryCallback, const CreateFileCallback& createFileCallback,
	  const ModifyDirectoryCallback& modifyDirectoryCallback, const ModifyFileCallback& modifyFileCallback) const
	{
		i32 removePathIndex = 0;
		i32 renamePathIndex = 0;
		i32 createDirectoryIndex = 0;
		i32 createFileIndex = 0;
		i32 modifyDirectoryIndex = 0;
		i32 modifyFileIndex = 0;
		foreach(const auto & type, m_typeOrder)
		{
			switch (type)
			{
			case RemovePath:
				removePathCallback(m_removedPathes[removePathIndex++]);
				break;
			case RenamePath:
				renamePathCallback(m_renamedPathes[renamePathIndex].keyAbsolutePath, m_renamedPathes[renamePathIndex].toName);
				renamePathIndex++;
				break;
			case CreateDirectory:
				createDirectoryCallback(m_createdDirectories[createDirectoryIndex++]);
				break;
			case CreateFile:
				createFileCallback(m_createdFiles[createFileIndex++]);
				break;
			case ModifyDirectory:
				modifyDirectoryCallback(m_modifiedDirectories[modifyDirectoryIndex++]);
				break;
			case ModifyFile:
				modifyFileCallback(m_modifiedFiles[modifyFileIndex++]);
				break;
			default:
				DRX_ASSERT(!"Bad Type");
			}
		}
	}

	void Reset();

	void AddRemovePath(const QString& keyAbsolutePath);
	void AddRename(const QString& keyAbsolutePath, const SAbsolutePath& toName);
	void AddCreateDirectory(const SDirectoryInfoInternal&);
	void AddCreateFile(const SFileInfoInternal&);
	void AddModifyDirectory(const SDirectoryInfoInternal&);
	void AddModifyFile(const SFileInfoInternal&);

private:
	struct SPathRename
	{
		QString       keyAbsolutePath;
		SAbsolutePath toName;
	};
	enum Types
	{
		RemovePath,
		RenamePath,
		CreateDirectory,
		CreateFile,
		ModifyFile,
		ModifyDirectory
	};

	QVector<Types>                  m_typeOrder;
	QVector<QString>                m_removedPathes;
	QVector<SPathRename>            m_renamedPathes;
	QVector<SDirectoryInfoInternal> m_createdDirectories;
	QVector<SFileInfoInternal>      m_createdFiles;
	QVector<SDirectoryInfoInternal> m_modifiedDirectories;
	QVector<SFileInfoInternal>      m_modifiedFiles;
};

} //endns Internal
} //endns FileSystem

Q_DECLARE_METATYPE(FileSystem::Internal::CUpdateSequence)

