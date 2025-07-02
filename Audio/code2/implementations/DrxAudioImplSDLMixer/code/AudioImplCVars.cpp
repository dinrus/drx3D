// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../AudioImplCVars.h"
#include <drx3D/Sys/IConsole.h>

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
//////////////////////////////////////////////////////////////////////////
void CCVars::RegisterVariables()
{
}

//////////////////////////////////////////////////////////////////////////
void CCVars::UnregisterVariables()
{
	IConsole* const pConsole = gEnv->pConsole;

	if (pConsole != nullptr)
	{
	}
}
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
