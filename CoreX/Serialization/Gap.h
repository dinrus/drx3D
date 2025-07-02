// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>

namespace Serialization
{
// used to fill the property rows by empty space
struct SGap
{
	SGap() : width(-1) {}
	explicit SGap(i32 w) : width(w) {}

	void Serialize(Serialization::IArchive& ar) {}

	i32  width;
};
}
