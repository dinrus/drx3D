// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../Common.h"

namespace DrxAudio
{
namespace Impl
{
namespace Wwise
{
AkGameObjectID g_listenerId = AK_INVALID_GAME_OBJECT; // To be removed once multi-listener support is implemented.
AkGameObjectID g_globalObjectId = AK_INVALID_GAME_OBJECT;
} //endns Wwise
} //endns Impl
} //endns DrxAudio
