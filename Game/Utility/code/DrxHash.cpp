// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/DrxHash.h>

DrxHashStringId DrxHashStringId::GetIdForName( tukk _name )
{
	DRX_ASSERT(_name);

	return DrxHashStringId(_name);
}
