// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/VoiceListener.h>

void CGameContext::RegisterExtensions(IGameFramework* pFW)
{
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	REGISTER_FACTORY(pFW, "VoiceListener", CVoiceListener, false);
#endif
}
