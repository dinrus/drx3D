// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <DrxSystem/File/IDrxPak.h>
#include <drx3D/CoreX/String/DrxPath.h>
#include "DBATable.h"
#include "Serialization.h"
#include <drx3D/CoreX/Serialization/yasli/JSONIArchive.h>
#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>
#include <DrxSystem/XML/IXml.h>

void SDBAEntry::Serialize(IArchive& ar)
{
	if (ar.isEdit() && ar.isOutput())
		ar(path, "digest", "!^");
	ar(OutputFilePath(path, "Animation Databases (.dba)|*.dba", "Animations"), "path", "<Path");
	filter.Serialize(ar);
}

void SDBATable::Serialize(IArchive& ar)
{
	ar(entries, "entries", "Entries");
}

bool SDBATable::Load(tukk fullPath)
{
	yasli::JSONIArchive ia;
	if (!ia.load(fullPath))
		return false;

	ia(*this);

	return true;
}

bool SDBATable::Save(tukk fullPath)
{
	yasli::JSONOArchive oa(120);
	oa(*this);

	oa.save(fullPath);
	return true;
}

i32 SDBATable::FindDBAForAnimation(const SAnimationFilterItem& animation) const
{
	for (size_t i = 0; i < entries.size(); ++i)
		if (entries[i].filter.MatchesFilter(animation))
			return i32(i);

	return -1;
}

