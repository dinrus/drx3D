// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CLIPS_POSITIONING__H__
#define __PROCEDURAL_CLIPS_POSITIONING__H__

#include <drx3D/Act/IDrxMannequin.h>

struct SProceduralClipPosAdjustTargetLocatorParams
	: public IProceduralParams
{
	SProceduralClipPosAdjustTargetLocatorParams()
	{
	}

	virtual void Serialize(Serialization::IArchive& ar) override;

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = targetScopeName.c_str();
	}

	SProcDataCRC    targetScopeName;
	TProcClipString targetJointName;
	SProcDataCRC    targetStateName;
};

#endif
