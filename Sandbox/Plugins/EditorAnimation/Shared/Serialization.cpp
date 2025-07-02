// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "Serialization.h"
#include "AnimSettings.h"

bool Serialize(Serialization::IArchive& ar, SkeletonAlias& value, tukk name, tukk label)
{
	return ar(value.alias, name, label);
}

