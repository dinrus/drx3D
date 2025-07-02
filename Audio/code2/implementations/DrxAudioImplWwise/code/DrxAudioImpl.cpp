// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../AudioImpl.h"
#include "../AudioImplCVars.h"
#include <drx3D/Audio/Logger.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#if DRX_PLATFORM_DURANGO
	#include <apu.h>
	#include <shapexmacontext.h>
#endif // DRX_PLATFORM_DURANGO

namespace DrxAudio
{
namespace Impl
{
namespace Wwise
{
// Define global objects.
CCVars g_cvars;

#if defined(PROVIDE_WWISE_IMPL_SECONDARY_POOL)
MemoryPoolReferenced g_audioImplMemoryPoolSecondary;
#endif // PROVIDE_AUDIO_IMPL_SECONDARY_POOL

//////////////////////////////////////////////////////////////////////////
class CEngineModule_DrxAudioImplWwise : public DrxAudio::IImplModule
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_ADD(DrxAudio::IImplModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DrxAudioImplWwise, "EngineModule_AudioImpl", "b4971e5d-d024-42c5-b34a-9ac0b4abfffd"_drx_guid)

	CEngineModule_DrxAudioImplWwise();

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override     { return "DrxAudioImplWwise"; }
	virtual tukk GetCategory() const override { return "DrxAudio"; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
#if defined(PROVIDE_WWISE_IMPL_SECONDARY_POOL)
		size_t secondarySize = 0;
		uk pSecondaryMemory = nullptr;

	#if DRX_PLATFORM_DURANGO
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Wwise Implementation Audio Pool Secondary");
		secondarySize = g_cvars.m_secondaryMemoryPoolSize << 10;

		APU_ADDRESS temp;
		HRESULT const result = ApuAlloc(&pSecondaryMemory, &temp, secondarySize, SHAPE_XMA_INPUT_BUFFER_ALIGNMENT);
		DRX_ASSERT(result == S_OK);
	#endif  // DRX_PLATFORM_DURANGO

		g_audioImplMemoryPoolSecondary.InitMem(secondarySize, (u8*)pSecondaryMemory);
#endif    // PROVIDE_AUDIO_IMPL_SECONDARY_POOL

		gEnv->pAudioSystem->AddRequestListener(&CEngineModule_DrxAudioImplWwise::OnAudioEvent, nullptr, ESystemEvents::ImplSet);
		SRequestUserData const data(ERequestFlags::ExecuteBlocking | ERequestFlags::CallbackOnExternalOrCallingThread);
		gEnv->pAudioSystem->SetImpl(new CImpl, data);
		gEnv->pAudioSystem->RemoveRequestListener(&CEngineModule_DrxAudioImplWwise::OnAudioEvent, nullptr);

		if (m_bSuccess)
		{
			Drx::Audio::Log(ELogType::Always, "DrxAudioImplWwise loaded");
		}
		else
		{
			Drx::Audio::Log(ELogType::Error, "DrxAudioImplWwise failed to load");
		}

		return m_bSuccess;
	}

	//////////////////////////////////////////////////////////////////////////
	static void OnAudioEvent(SRequestInfo const* const pAudioRequestInfo)
	{
		m_bSuccess = pAudioRequestInfo->requestResult == ERequestResult::Success;
	}

	static bool m_bSuccess;
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DrxAudioImplWwise)
bool CEngineModule_DrxAudioImplWwise::m_bSuccess = false;

CEngineModule_DrxAudioImplWwise::CEngineModule_DrxAudioImplWwise()
{
	g_cvars.RegisterVariables();
}
} //endns Wwise
} //endns Impl
} //endns DrxAudio
#include <drx3D/CoreX/CrtDebugStats.h>
