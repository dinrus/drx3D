// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "FileType.h"

#include <QCoreApplication>

tukk SFileType::trContext = "FileType";

QString SFileType::name() const
{
	return QCoreApplication::translate(trContext, nameTrKey);
}

const SFileType* SFileType::Unknown()
{
	static const SFileType s_unknown = []()
	{
		SFileType unknown;
		unknown.nameTrKey = "Unknown file";
		unknown.iconPath = QStringLiteral("icons:General/File.ico");
		return unknown;
	} ();
	return &s_unknown;
}

const SFileType* SFileType::DirectoryType()
{
	static const SFileType sDirectoryType = []()
	{
		SFileType directoryType;
		directoryType.nameTrKey = "File folder";
		directoryType.iconPath = QStringLiteral("icons:General/Folder.ico");
		return directoryType;
	} ();
	return &sDirectoryType;
}

const SFileType* SFileType::SymLink()
{
	static const SFileType sSymLink = []()
	{
		SFileType directoryType;
		directoryType.nameTrKey = "SymLink";
		directoryType.iconPath = QStringLiteral("icons:General/SymLink.ico");
		return directoryType;
	} ();
	return &sSymLink;
}

void SFileType::CheckValid() const
{
	for (auto& folder : folders)
	{
		DRX_ASSERT(folder == folder.toLower());
	}
	DRX_ASSERT(primaryExtension == primaryExtension.toLower());
	for (auto& extension : extraExtensions)
	{
		DRX_ASSERT(extension == extension.toLower());
	}
}

