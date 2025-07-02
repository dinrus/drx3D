// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
class CCVars final
{
public:

	CCVars() = default;
	CCVars(CCVars const&) = delete;
	CCVars(CCVars&&) = delete;
	CCVars& operator=(CCVars const&) = delete;
	CCVars& operator=(CCVars&&) = delete;

	void    RegisterVariables();
	void    UnregisterVariables();
};

extern CCVars g_cvars;
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
