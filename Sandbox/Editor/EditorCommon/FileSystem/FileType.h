// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QString>
#include <QVector>
#include <QMetaType>

/**
 * \brief statically describes a known file type of engine
 *
 * this structure should be statically allocated and never freed
 */
struct SFileType
{
	static tukk      trContext; ///< context for nameKey

	tukk             nameTrKey;        ///< translatable name for the file type
	QString                 iconPath;         ///< icon path used for visualisations
	QVector<QString>        folders;          ///< engine folders this file type exists in (empty means all folders)
	QString                 primaryExtension; ///< main extension (default for opening & saving the file)
	QVector<QString>        extraExtensions;  ///< additional file name extensions

	QString                 name() const; ///< \returns name for current translation

	static const SFileType* Unknown();
	static const SFileType* DirectoryType();
	static const SFileType* SymLink();

	void                    CheckValid() const;
};

Q_DECLARE_METATYPE(const SFileType*)

