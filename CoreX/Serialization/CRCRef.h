// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

template<u32 StoreStrings, typename THash>
struct SCRCRef;

#include <drx3D/CoreX/Serialization/Forward.h>

template<u32 StoreStrings, typename THash>
bool Serialize(Serialization::IArchive& ar, SCRCRef<StoreStrings, THash>& crcRef, tukk name, tukk label);

#include "CRCRefImpl.h"
