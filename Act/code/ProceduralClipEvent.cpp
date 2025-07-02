// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/Act/Serialization.h>

struct SProceduralClipEventParams
	: public IProceduralParams
{
	virtual void Serialize(Serialization::IArchive& ar) override
	{
		ar(eventName, "EventName", "Event Name");
	}

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = eventName.c_str();
	}

	SProcDataCRC eventName;
};

class CProceduralClipEvent : public TProceduralClip<SProceduralClipEventParams>
{
public:
	CProceduralClipEvent()
	{
	}

	virtual void OnEnter(float blendTime, float duration, const SProceduralClipEventParams& params)
	{
		SendActionEvent(params.eventName.crc);
	}

	virtual void OnExit(float blendTime)  {}

	virtual void Update(float timePassed) {}

};

REGISTER_PROCEDURAL_CLIP(CProceduralClipEvent, "ActionEvent");
