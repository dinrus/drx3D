// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>

namespace DrxAudio
{
namespace Impl
{
struct IImpl;
struct SObject3DAttributes;
}

class CATLListener;

class CAudioListenerUpr final
{
public:

	CAudioListenerUpr() = default;
	~CAudioListenerUpr();

	CAudioListenerUpr(CAudioListenerUpr const&) = delete;
	CAudioListenerUpr(CAudioListenerUpr&&) = delete;
	CAudioListenerUpr&           operator=(CAudioListenerUpr const&) = delete;
	CAudioListenerUpr&           operator=(CAudioListenerUpr&&) = delete;

	void                             Init(Impl::IImpl* const pIImpl);
	void                             Release();
	void                             Update(float const deltaTime);
	CATLListener*                    CreateListener(char const* const szName = nullptr);
	void                             ReleaseListener(CATLListener* const pListener);
	size_t                           GetNumActiveListeners() const;
	Impl::SObject3DAttributes const& GetActiveListenerAttributes() const;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	char const* GetActiveListenerName() const;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

private:

	std::vector<CATLListener*> m_activeListeners;
	Impl::IImpl*               m_pIImpl = nullptr;
};
} //endns DrxAudio
