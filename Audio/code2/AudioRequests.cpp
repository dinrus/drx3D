// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Audio/StdAfx.h>
#include <drx3D/Audio/AudioInternalInterfaces.h>
#include <drx3D/Audio/Logger.h>

namespace DrxAudio
{
#define REQUEST_CASE_BLOCK(CLASS, ENUM, P_SOURCE, P_RESULT)                        \
  case ENUM:                                                                       \
    {                                                                              \
      P_RESULT = new CLASS<ENUM>(static_cast<CLASS<ENUM> const* const>(P_SOURCE)); \
                                                                                   \
      break;                                                                       \
    }

#define AM_REQUEST_BLOCK(ENUM)  REQUEST_CASE_BLOCK(SAudioUprRequestData, ENUM, pRequestData, pResult)
#define ACM_REQUEST_BLOCK(ENUM) REQUEST_CASE_BLOCK(SAudioCallbackUprRequestData, ENUM, pRequestData, pResult)
#define AO_REQUEST_BLOCK(ENUM)  REQUEST_CASE_BLOCK(SAudioObjectRequestData, ENUM, pRequestData, pResult)
#define AL_REQUEST_BLOCK(ENUM)  REQUEST_CASE_BLOCK(SAudioListenerRequestData, ENUM, pRequestData, pResult)

////////////////////////////////////////////////////////////////////////////
SAudioRequestData* AllocateRequestData(SAudioRequestData const* const pRequestData)
{
	DRX_ASSERT(pRequestData != nullptr);
	SAudioRequestData* pResult = nullptr;
	EAudioRequestType const requestType = pRequestData->type;

	switch (requestType)
	{
	case EAudioRequestType::AudioUprRequest:
		{
			SAudioUprRequestDataBase const* const pBase = static_cast<SAudioUprRequestDataBase const* const>(pRequestData);

			switch (pBase->type)
			{
				AM_REQUEST_BLOCK(EAudioUprRequestType::SetAudioImpl)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ReleaseAudioImpl)
				AM_REQUEST_BLOCK(EAudioUprRequestType::RefreshAudioSystem)
				AM_REQUEST_BLOCK(EAudioUprRequestType::StopAllSounds)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ParseControlsData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ParsePreloadsData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ClearControlsData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ClearPreloadsData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::PreloadSingleRequest)
				AM_REQUEST_BLOCK(EAudioUprRequestType::UnloadSingleRequest)
				AM_REQUEST_BLOCK(EAudioUprRequestType::UnloadAFCMDataByScope)
				AM_REQUEST_BLOCK(EAudioUprRequestType::DrawDebugInfo)
				AM_REQUEST_BLOCK(EAudioUprRequestType::AddRequestListener)
				AM_REQUEST_BLOCK(EAudioUprRequestType::RemoveRequestListener)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ChangeLanguage)
				AM_REQUEST_BLOCK(EAudioUprRequestType::RetriggerAudioControls)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ReleasePendingRays)
				AM_REQUEST_BLOCK(EAudioUprRequestType::ReloadControlsData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::GetAudioFileData)
				AM_REQUEST_BLOCK(EAudioUprRequestType::GetImplInfo)
			default:
				{
					Drx::Audio::Log(ELogType::Error, "Unknown audio manager request type (%u)", pBase->type);
					DRX_ASSERT(false);

					break;
				}
			}

			break;
		}
	case EAudioRequestType::AudioObjectRequest:
		{
			SAudioObjectRequestDataBase const* const pBase = static_cast<SAudioObjectRequestDataBase const* const>(pRequestData);

			switch (pBase->type)
			{
				AO_REQUEST_BLOCK(EAudioObjectRequestType::LoadTrigger)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::UnloadTrigger)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::PlayFile)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::StopFile)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::ExecuteTrigger)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::ExecuteTriggerEx)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::StopTrigger)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::StopAllTriggers)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetTransformation)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetParameter)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetSwitchState)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetCurrentEnvironments)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetEnvironment)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::ResetEnvironments)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::RegisterObject)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::ReleaseObject)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::ProcessPhysicsRay)
				AO_REQUEST_BLOCK(EAudioObjectRequestType::SetName)
			default:
				{
					Drx::Audio::Log(ELogType::Error, "Unknown audio object request type (%u)", pBase->type);
					DRX_ASSERT(false);

					break;
				}
			}
			break;
		}
	case EAudioRequestType::AudioListenerRequest:
		{
			SAudioListenerRequestDataBase const* const pBase = static_cast<SAudioListenerRequestDataBase const* const>(pRequestData);

			switch (pBase->type)
			{
				AL_REQUEST_BLOCK(EAudioListenerRequestType::SetTransformation)
				AL_REQUEST_BLOCK(EAudioListenerRequestType::RegisterListener)
				AL_REQUEST_BLOCK(EAudioListenerRequestType::ReleaseListener)
				AL_REQUEST_BLOCK(EAudioListenerRequestType::SetName)
			default:
				{
					Drx::Audio::Log(ELogType::Error, "Unknown audio listener request type (%u)", pBase->type);
					DRX_ASSERT(false);

					break;
				}
			}

			break;
		}
	case EAudioRequestType::AudioCallbackUprRequest:
		{
			SAudioCallbackUprRequestDataBase const* const pBase = static_cast<SAudioCallbackUprRequestDataBase const* const>(pRequestData);

			switch (pBase->type)
			{
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportStartedEvent)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportFinishedEvent)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportFinishedTriggerInstance)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportStartedFile)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportStoppedFile)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportVirtualizedEvent)
				ACM_REQUEST_BLOCK(EAudioCallbackUprRequestType::ReportPhysicalizedEvent)
			default:
				{
					Drx::Audio::Log(ELogType::Error, "Unknown audio callback manager request type (%u)", pBase->type);
					DRX_ASSERT(false);

					break;
				}
			}

			break;
		}
	default:
		{
			Drx::Audio::Log(ELogType::Error, "Unknown audio request type (%u)", requestType);
			DRX_ASSERT(false);

			break;
		}
	}

	return pResult;
}
} //endns DrxAudio
