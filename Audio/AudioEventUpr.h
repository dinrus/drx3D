// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	#include <drx3D/CoreX/Math/Drx_Math.h>
struct IRenderAuxGeom;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

namespace DrxAudio
{
class CATLEvent;

namespace Impl
{
struct IImpl;
} //endns Impl

class CAudioEventUpr final
{
public:

	CAudioEventUpr() = default;
	~CAudioEventUpr();

	CAudioEventUpr(CAudioEventUpr const&) = delete;
	CAudioEventUpr& operator=(CAudioEventUpr const&) = delete;

	void                Init(Impl::IImpl* const pIImpl);
	void                Release();

	CATLEvent*          ConstructAudioEvent();
	void                ReleaseEvent(CATLEvent* const pEvent);

	size_t              GetNumConstructed() const;

private:

	std::list<CATLEvent*> m_constructedAudioEvents;
	Impl::IImpl*          m_pIImpl = nullptr;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void DrawDebugInfo(IRenderAuxGeom& auxGeom, Vec3 const& listenerPosition, float const posX, float posY) const;

#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};
} //endns DrxAudio
