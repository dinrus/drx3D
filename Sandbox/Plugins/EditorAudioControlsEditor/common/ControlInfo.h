// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SharedData.h"
#include <DrxIcon.h>

namespace ACE
{
struct SControlInfo
{
	SControlInfo(string const& name_, ControlId const id_, DrxIcon const& icon_)
		: name(name_)
		, id(id_)
		, icon(icon_)
	{}

	string const&   name;
	ControlId const id;
	DrxIcon const&  icon;
};

using SControlInfos = std::vector<SControlInfo>;
} //endns ACE
