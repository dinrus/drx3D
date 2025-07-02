// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileType.h"

#include <QVector>
#include <QMultiHash>

/// \brief stores registered file types
class CFileTypeStore
{
public:
	/// \return file type for the described file
	/// \note always returns a valid file type
	const SFileType* GetType(const QString& keyEnginePath, const QString& extension) const;

	void             RegisterFileType(const SFileType* fileType);
	void             RegisterFileTypes(const QVector<const SFileType*>& fileTypes);

private:
	void AddExtensionFileType(const QString& extension, const SFileType* fileType);

private:
	QHash<QString, QVector<const SFileType*>> m_extensionTypes;
};

