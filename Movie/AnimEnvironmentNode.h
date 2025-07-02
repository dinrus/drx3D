// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIMENVIRONMENTNODE_H__
#define __ANIMENVIRONMENTNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CAnimEnvironmentNode : public CAnimNode
{
public:
	CAnimEnvironmentNode(i32k id);
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Environment; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           Activate(bool bActivate) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

private:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;
	virtual void InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override;

	void         StoreCelestialPositions();
	void         RestoreCelestialPositions();

	float m_oldSunLongitude;
	float m_oldSunLatitude;
	float m_oldMoonLongitude;
	float m_oldMoonLatitude;
};

#endif
