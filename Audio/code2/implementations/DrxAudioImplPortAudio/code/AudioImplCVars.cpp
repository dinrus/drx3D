// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../AudioImplCVars.h"
#include <drx3D/Sys/IConsole.h>

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
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
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
