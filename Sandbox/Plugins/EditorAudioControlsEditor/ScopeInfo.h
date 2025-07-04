// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>

namespace ACE
{
// Available levels where the controls can be stored.
struct SScopeInfo
{
	SScopeInfo(string const& name_ = "", bool const isLocalOnly_ = false)
		: name(name_)
		, isLocalOnly(isLocalOnly_)
	{}

	string name;

	// If true, there is a level in the game audio data that doesn't
	// exist in the global list of levels for your project.
	bool isLocalOnly;
};

using ScopeInfos = std::vector<SScopeInfo>;
} //endns ACE
