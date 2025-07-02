// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AnimationFilter.h"
#include <drx3D/CoreX/Serialization/Forward.h>

struct SDBAEntry
{
	SAnimationFilter filter;
	string           path;

	void             Serialize(Serialization::IArchive& ar);
};

struct SDBATable
{
	std::vector<SDBAEntry> entries;

	void                   Serialize(Serialization::IArchive& ar);

	// returns -1 when nothing is found
	i32  FindDBAForAnimation(const SAnimationFilterItem& animation) const;

	bool Load(tukk dbaTablePath);
	bool Save(tukk dbaTablePath);
};

