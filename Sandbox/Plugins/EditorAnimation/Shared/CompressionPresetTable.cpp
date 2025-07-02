// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <drx3D/CoreX/Platform/platform.h>
#include "CompressionPresetTable.h"
#include "AnimationFilter.h"
#include "DBATable.h"
#include "Serialization.h"
#include <drx3D/CoreX/Serialization/yasli/JSONIArchive.h>
#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>

void SCompressionPresetEntry::Serialize(Serialization::IArchive& ar)
{
	if (ar.isEdit() && ar.isOutput())
		ar(name, "digest", "!^");
	ar(name, "name", "Name");
	filter.Serialize(ar);
	if (!filter.condition)
		ar.warning(filter.condition, "Specify filter to apply preset to animations.");
	ar(settings, "settings", "+Settings");
}

void SCompressionPresetTable::Serialize(Serialization::IArchive& ar)
{
	ar(entries, "entries", "Entries");
}

bool SCompressionPresetTable::Load(tukk fullPath)
{
	yasli::JSONIArchive ia;
	if (!ia.load(fullPath))
		return false;

	ia(*this);
	return true;
}

bool SCompressionPresetTable::Save(tukk fullPath)
{
	yasli::JSONOArchive oa(120);
	oa(*this);

	return oa.save(fullPath);
}

const SCompressionPresetEntry* SCompressionPresetTable::FindPresetForAnimation(tukk animationPath, const vector<string>& tags, tukk skeletonAlias) const
{
	SAnimationFilterItem item;
	item.path = animationPath;
	item.tags = tags;
	item.skeletonAlias = skeletonAlias;

	for (size_t i = 0; i < entries.size(); ++i)
	{
		const SCompressionPresetEntry& entry = entries[i];
		if (entry.filter.MatchesFilter(item))
			return &entry;
	}

	return 0;
}

