// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Audio/StdAfx.h>
#include <drx3D/Audio/AudioEventListenerUpr.h>
#include <drx3D/Audio/Logger.h>

namespace DrxAudio
{
//////////////////////////////////////////////////////////////////////////
CAudioEventListenerUpr::~CAudioEventListenerUpr()
{
	stl::free_container(m_listeners);
}

//////////////////////////////////////////////////////////////////////////
ERequestStatus CAudioEventListenerUpr::AddRequestListener(SAudioUprRequestData<EAudioUprRequestType::AddRequestListener> const* const pRequestData)
{
	ERequestStatus result = ERequestStatus::Failure;

	for (auto const& listener : m_listeners)
	{
		if (listener.OnEvent == pRequestData->func && listener.pObjectToListenTo == pRequestData->pObjectToListenTo
		    && listener.eventMask == pRequestData->eventMask)
		{
			result = ERequestStatus::Success;
			break;
		}
	}

	if (result == ERequestStatus::Failure)
	{
		SAudioEventListener audioEventListener;
		audioEventListener.pObjectToListenTo = pRequestData->pObjectToListenTo;
		audioEventListener.OnEvent = pRequestData->func;
		audioEventListener.eventMask = pRequestData->eventMask;
		m_listeners.push_back(audioEventListener);
		result = ERequestStatus::Success;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
ERequestStatus CAudioEventListenerUpr::RemoveRequestListener(void (* func)(SRequestInfo const* const), void const* const pObjectToListenTo)
{
	ERequestStatus result = ERequestStatus::Failure;
	ListenerArray::iterator Iter(m_listeners.begin());
	ListenerArray::const_iterator const IterEnd(m_listeners.end());

	for (; Iter != IterEnd; ++Iter)
	{
		if ((Iter->OnEvent == func || func == nullptr) && Iter->pObjectToListenTo == pObjectToListenTo)
		{
			if (Iter != IterEnd - 1)
			{
				(*Iter) = m_listeners.back();
			}

			m_listeners.pop_back();
			result = ERequestStatus::Success;

			break;
		}
	}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (result == ERequestStatus::Failure)
	{
		Drx::Audio::Log(ELogType::Warning, "Failed to remove a request listener!");
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return result;
}

//////////////////////////////////////////////////////////////////////////
void CAudioEventListenerUpr::NotifyListener(SRequestInfo const* const pResultInfo)
{
	DRX_PROFILE_FUNCTION(PROFILE_AUDIO);

	for (auto const& listener : m_listeners)
	{
		if (((listener.eventMask & pResultInfo->systemEvent) > 0)                                            //check: is the listener interested in this specific event?
		    && (listener.pObjectToListenTo == nullptr || listener.pObjectToListenTo == pResultInfo->pOwner)) //check: is the listener interested in events from this sender
		{
			listener.OnEvent(pResultInfo);
		}
	}
}
} //endns DrxAudio
