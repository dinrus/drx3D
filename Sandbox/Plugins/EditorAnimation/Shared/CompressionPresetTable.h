// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AnimationFilter.h"
#include "AnimSettings.h"

struct SCompressionPresetEntry
{
	string               name;
	SAnimationFilter     filter;
	SCompressionSettings settings;

	void                 Serialize(Serialization::IArchive& ar);
};

struct SCompressionPresetTable
{
	std::vector<SCompressionPresetEntry> entries;

	const SCompressionPresetEntry* FindPresetForAnimation(tukk animationPath, const vector<string>& tags, tukk skeletonAlias) const;

	bool Load(tukk tablePath);
	bool Save(tukk tablePath);
	void Serialize(Serialization::IArchive& ar);
};

