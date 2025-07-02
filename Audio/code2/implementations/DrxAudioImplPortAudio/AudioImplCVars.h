// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
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
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
