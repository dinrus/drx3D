// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace DrxAudio
{
namespace Impl
{
namespace Wwise
{
class CImpl;

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
	void    SetImpl(CImpl* const pImpl) { s_pImpl = pImpl; }

	i32 m_secondaryMemoryPoolSize = 0;
	i32 m_prepareEventMemoryPoolSize = 0;
	i32 m_streamUprMemoryPoolSize = 0;
	i32 m_streamDeviceMemoryPoolSize = 0;
	i32 m_soundEngineDefaultMemoryPoolSize = 0;
	i32 m_commandQueueMemoryPoolSize = 0;
	i32 m_lowerEngineDefaultPoolSize = 0;
	i32 m_enableEventUprThread = 0;
	i32 m_enableSoundBankUprThread = 0;
	i32 m_panningRule = 0;

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	i32 m_enableCommSystem = 0;
	i32 m_enableOutputCapture = 0;
	i32 m_monitorMemoryPoolSize = 0;
	i32 m_monitorQueueMemoryPoolSize = 0;
#endif  // INCLUDE_WWISE_IMPL_PRODUCTION_CODE

	static CImpl* s_pImpl;
};

extern CCVars g_cvars;
} //endns Wwise
} //endns Impl
} //endns DrxAudio
