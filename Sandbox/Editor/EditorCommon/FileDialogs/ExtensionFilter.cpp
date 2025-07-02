// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "ExtensionFilter.h"

#include <QFileInfo>

ExtensionFilterVector CExtensionFilter::Parse(tukk pattern)
{
	ExtensionFilterVector result;

	const auto filterEntries = QString::fromLocal8Bit(pattern).split(QChar('|'), QString::SkipEmptyParts);

	DRX_ASSERT_MESSAGE(filterEntries.size() % 2 == 0, "Invalid extension format");

	i32k count = filterEntries.size() / 2;
	for (i32 i = 0; i < count; i++)
	{
		auto description = filterEntries[i*2];
		auto fileSuffixes = filterEntries[i*2 + 1].split(QChar(';'), QString::SkipEmptyParts);
		for (auto& suffix : fileSuffixes)
		{
			suffix = QFileInfo(suffix).completeSuffix();
		}

		result << CExtensionFilter(description, fileSuffixes);
	}
	return result;
}

